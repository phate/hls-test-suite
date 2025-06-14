import pprint
import re
import math

import vcdlib
import dataclasses
import typing
import collections


@dataclasses.dataclass
class ReadReq:
    ts: int
    addr: int
    id: int
    size: int


@dataclasses.dataclass
class WriteReq(ReadReq):
    data: int


@dataclasses.dataclass
class Res:
    ts: int
    id: int
    data: int


class Mem:
    def _req(self, ts):
        if self["req_ready"][ts] and self["req_valid"][ts]:
            if self.has_write and self["req_data_write"][ts]:
                return WriteReq(ts, self["req_data_addr"][ts] - self.addr_offset, self["req_data_id"][ts],
                                self["req_data_size"][ts],
                                self["req_data_data"][ts])
            else:
                return ReadReq(ts, self["req_data_addr"][ts] - self.addr_offset, self["req_data_id"][ts],
                               self["req_data_size"][ts])
        return None

    def _res(self, ts):
        if self["res_ready"][ts] and self["res_valid"][ts]:
            return Res(ts, self["res_data_id"][ts], self["res_data_data"][ts])
        return None

    def __getitem__(self, item: str):
        return self.vcd[("TOP", f"{self.name}_{item}")]

    def __init__(self, vcd: vcdlib.Vcd, name: str, addr_offset: int = 0):
        self.vcd = vcd
        self.name = name
        self.reqs = []
        self.ress = []
        self.addr_offset = addr_offset
        self.has_write = ("TOP", f"{self.name}_req_data_write") in self.vcd
        for ts in range(vcd.start_time, vcd.last_change, vcd.time_step * 2):
            if r := self._req(ts):
                self.reqs.append(r)
            if r := self._res(ts):
                self.ress.append(r)


@dataclasses.dataclass
class PortValue:
    ts: int
    data: int


def find_name_path(search_name: str, hier):
    for name, h in hier.items():
        if name == search_name:
            return (name,)
        if path := find_name_path(search_name, h):
            return (name,) + path


class Node:
    def __getitem__(self, item: str):
        return self.vcd[self.node_path + (item,)]

    def __contains__(self, item):
        return self.node_path + (item,) in self.vcd.net_dict

    def _port(self, port, ts):
        if self[f"{port}_ready"][ts] and self[f"{port}_valid"][ts]:
            return PortValue(ts, self[f"{port}_data"][ts])
        return None

    def _get_ports(self, direction: str):
        hier = self.vcd.net_hier
        for n in self.node_path:
            hier = hier[n]
        result = []
        for i in range(200):
            if f"{direction}{i}_ready" in hier:
                result.append(f"{direction}{i}")
        return result

    def __init__(self, vcd: vcdlib.Vcd, name: str):
        self.vcd = vcd
        self.name = name
        self.node_path = find_name_path(name, vcd.net_hier)
        ins = self._get_ports("i")
        outs = self._get_ports("o")
        self.i = []
        self.o = []
        for i in ins:
            tmp = []
            # TODO: find better fix for bundles
            if f"{i}_data" in self:
                for ts in range(vcd.start_time, vcd.last_change, vcd.time_step * 2):
                    if p := self._port(i, ts):
                        tmp.append(p)
            self.i.append(tmp)
        for o in outs:
            tmp = []
            # TODO: find better fix for bundles
            if f"{o}_data" in self:
                for ts in range(vcd.start_time, vcd.last_change, vcd.time_step * 2):
                    if p := self._port(o, ts):
                        tmp.append(p)
            self.o.append(tmp)


def match_mem_request(a: ReadReq, b: ReadReq):
    assert a.addr == b.addr
    assert a.id == b.id
    assert a.size == b.size
    if isinstance(a, WriteReq):
        assert a.data == b.data
    else:
        assert not isinstance(b, WriteReq)


def match_mem_requests(a: typing.List[ReadReq], b: typing.List[ReadReq]):
    for aa, bb in zip(a, b):
        match_mem_request(aa, bb)
    # check contents first to detect earlier mismatch
    assert len(a) == len(b)


def match_mem_response(a: Res, b: Res):
    assert a.id == b.id
    assert a.data == b.data


def match_mem_responses(a: typing.List[Res], b: typing.List[Res]):
    for aa, bb in zip(a, b):
        match_mem_response(aa, bb)
    # check contents first to detect earlier mismatch
    assert len(a) == len(b)


def match_node_io(a: typing.List[typing.List[PortValue]], b: typing.List[typing.List[PortValue]]):
    rot_a = list(zip(*a))
    rot_b = list(zip(*b))
    # this assumes for now that all ports have same number of transactions - wrong for e.g. LoopBuf
    for aa, bb in zip(rot_a, rot_b):
        for aaa, bbb in zip(aa, bb):
            assert aaa.data == bbb.data
    for aa in a:
        assert len(aa) == len(a[0])
    for bb in b:
        assert len(bb) == len(a[0])


def get_inputs(vcd: vcdlib.Vcd):
    kernel_path = find_name_path("sr", vcd.net_hier)[:-1]
    hier = vcd.net_hier
    for n in kernel_path:
        hier = hier[n]
    ready = vcd.net_dict[kernel_path + ("i_ready",)]
    valid = vcd.net_dict[kernel_path + ("i_valid",)]
    args = []
    for i in range(200):
        if f"i_data_{i}" not in hier:
            break
        args.append(vcd.net_dict[kernel_path + (f"i_data_{i}",)])
    for ts in range(vcd.start_time, vcd.last_change, vcd.time_step * 2):
        if ready[ts] and valid[ts]:
            return [a[ts] for a in args]

def find_op_names(hier, name_contains: str):
    res = []
    for name, h in hier.items():
        if name_contains in name and "ready" not in name and "valid" not in name and "data" not in name:
            res.append(name)
        res.extend(find_op_names(h, name_contains))
    return res


def find_stream_issue():
    # compares two different designs that should have the same behavior at certain known points (memory, distinct ops...)
    vcd_1 = vcdlib.Vcd("../build/decoupled/Vspmv_simple.vcd")
    vcd_2 = vcdlib.Vcd("../build/decoupled/Vspmv_stream.vcd")

    # get inputs so we have memory offsets
    inputs_1 = get_inputs(vcd_1)
    inputs_2 = get_inputs(vcd_2)

    vals_1 = Mem(vcd_1, "mem_0", inputs_1[0])
    cols_1 = Mem(vcd_1, "mem_1", inputs_1[1])
    rows_1 = Mem(vcd_1, "mem_2", inputs_1[2])
    vec_1 = Mem(vcd_1, "mem_3", inputs_1[6])
    out_1 = Mem(vcd_1, "mem_4", inputs_1[7])

    vals_2 = Mem(vcd_2, "mem_0", inputs_2[0])
    cols_2 = Mem(vcd_2, "mem_1", inputs_2[1])
    rows_2 = Mem(vcd_2, "mem_2", inputs_2[2])
    vec_2 = Mem(vcd_2, "mem_3", inputs_2[6])
    out_2 = Mem(vcd_2, "mem_4", inputs_2[7])

    fmul_1 = Node(vcd_1, find_op_names(vcd_1, "op_FPOP_mul")[0])
    fmul_2 = Node(vcd_2, find_op_names(vcd_2, "op_FPOP_mul")[0])

    fadd_1 = Node(vcd_1, find_op_names(vcd_1, "op_FPOP_add")[0])
    fadd_2 = Node(vcd_2, find_op_names(vcd_2, "op_FPOP_add")[0])

    match_mem_requests(vals_1.reqs, vals_2.reqs)
    match_mem_requests(cols_1.reqs, cols_2.reqs)
    match_mem_requests(rows_1.reqs, rows_2.reqs)
    match_mem_requests(vec_1.reqs, vec_2.reqs)

    match_mem_responses(vals_1.ress, vals_2.ress)
    match_mem_responses(cols_1.ress, cols_2.ress)
    match_mem_responses(rows_1.ress, rows_2.ress)
    match_mem_responses(vec_1.ress, vec_2.ress)

    match_node_io(fmul_1.i, fmul_2.i)
    match_node_io(fmul_1.o, fmul_2.o)

    match_node_io(fadd_1.i, fadd_2.i)
    match_node_io(fadd_1.o, fadd_2.o)

    match_mem_requests(out_1.reqs, out_2.reqs)
    match_mem_responses(out_1.ress, out_2.ress)


def find_earliest_divergence():
    vcd_1 = vcdlib.Vcd("../build/decoupled/binsearch_decouple_rif_while.upsize.hls.vcd")
    vcd_2 = vcdlib.Vcd("../scripts/Vbinsearch_decouple_rif_while.resize.vcd")

    nodes_1 = find_op_names(vcd_1.net_hier, "op_")
    nodes_2 = find_op_names(vcd_2.net_hier, "op_")
    common_nodes = sorted(set(nodes_1).intersection(nodes_2))
    print(common_nodes)
    divergences = []
    for name in common_nodes:
        if divergence := find_divergence(name, vcd_1, vcd_2):
            divergences.append(divergence)
    divergences = sorted(divergences)
    pprint.pprint(divergences)
    pass


def find_divergence(name, vcd_1, vcd_2):
    node_1 = Node(vcd_1, name)
    node_2 = Node(vcd_2, name)
    divergence = (vcd_1.last_time, vcd_2.last_time, "io", 0, 0)
    found = False
    for ix, (i1, i2) in enumerate(zip(node_1.i, node_2.i)):
        for ii1, ii2 in zip(i1, i2):
            if ii1.ts > divergence[0]:
                break
            if ii1.data != ii2.data:
                divergence = ii1.ts, ii2.ts, f"{name}.i{ix}", ii1.data, ii2.data
                found = True
                break
        if len(i1) > len(i2):
            first_missing = i1[len(i2)]
            if first_missing.ts < divergence[0]:
                found = True
                divergence = first_missing.ts, -1, f"{name}.i{ix}", first_missing.data, -1
    for ix, (i1, i2) in enumerate(zip(node_1.o, node_2.o)):
        for ii1, ii2 in zip(i1, i2):
            if ii1.ts > divergence[0]:
                break
            if ii1.data != ii2.data:
                divergence = ii1.ts, ii2.ts, f"{name}.o{ix}", ii1.data, ii2.data
                found = True
                break
        if len(i1) > len(i2):
            first_missing = i1[len(i2)]
            if first_missing.ts < divergence[0]:
                found = True
                divergence = first_missing.ts, -1, f"{name}.o{ix}", first_missing.data, -1
    if found:
        return divergence

def analyze_binsearch_simple_early():
    # finds the results for
    vcd = vcdlib.Vcd("../cmake-build-debug/Vbinsearch_simple_early.vcd")

    # get inputs so we have memory offsets
    inputs = get_inputs(vcd)

    table = Mem(vcd, "mem_0", inputs[0])
    sorted = Mem(vcd, "mem_1", inputs[1])
    result = Mem(vcd, "mem_2", inputs[2])

    # check accesses to sorted
    for i, (req, res) in enumerate(zip(sorted.reqs, sorted.ress)):
        expected_value = req.addr//4*2
        actual_value = res.data
        assert expected_value == actual_value

def find_hashtable_bug():
    vcd = vcdlib.Vcd("../cmake-build-debug/Vhashtable_decouple3.vcd")
    nodes = sorted(find_op_names(vcd.net_hier, "op_"))
    ts1 = 3172
    ts2 = 3176
    for name in nodes:
        if "HLS_MEM_REQ" in name:
            continue
        node = Node(vcd, name)
        for op in node._get_ports("o"):
            p1 =  node._port(op, ts1)
            p2 =  node._port(op, ts2)
            if (p1 is None) != (p2 is None):
                print(name, op, p1, p2)


def main():
    find_hashtable_bug()
    # find_earliest_divergence()
    # find_stream_issue()
    # analyze_binsearch_simple_early()

if __name__ == '__main__':
    main()
