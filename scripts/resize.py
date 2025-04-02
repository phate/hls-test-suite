import collections
import dataclasses
import math
import pprint
import re
import shutil
import tempfile

import vcdlib
import subprocess
import pathlib


@dataclasses.dataclass
class RhlsBuf:
    depth: int
    width: int
    pass_through: bool
    id: int = 0
    def to_chisel(self):
        return f"emitVerilog(new RhlsBuf({self.depth}, {self.width}, {str(self.pass_through).lower()}))"

    @staticmethod
    def from_instance(buf: str):
        match = re.match(r"op_HLS_BUF_(?P<pass_through>P_)?(?P<depth>\d+)_IN1_W(?P<width>\d+)_OUT1_W(?P=width)_(?P<id>\d+)", buf).groupdict()
        pass_through = bool(match["pass_through"])
        depth = int(match["depth"])
        width = int(match["width"])
        id = int(match["id"])
        return RhlsBuf(depth, width, pass_through, id)

    def to_module(self):
        return f"op_HLS_BUF{'_P' if self.pass_through else ''}_{self.depth}_I{self.width}W_O{self.width}W"

    def to_instance(self):
        return f"op_HLS_BUF{'_P' if self.pass_through else ''}_{self.depth}_IN1_W{self.width}_OUT1_W{self.width}_{self.id}"


@dataclasses.dataclass
class RhlsDecLoad:
    depth: int
    width: int
    addr_width: int
    type_name: str
    id: int = 0
    def to_chisel(self):
        return f'emitVerilog(new RhlsDecLoad({self.depth}, {self.width}, {self.addr_width}, "{self.type_name}"))'

    @staticmethod
    def from_instance(buf: str):
        match = re.match(r"op_HLS_DEC_LOAD_(?P<depth>\d+)_(?P<type_name>[^_]+)_IN2_W(?P<width>\d+)_OUT2_W(?P<addr_width>\d+)_(?P<id>\d+)", buf).groupdict()
        depth = int(match["depth"])
        width = int(match["width"])
        addr_width = int(match["addr_width"])
        type_name = match["type_name"]
        id = int(match["id"])
        return RhlsDecLoad(depth, width, addr_width, type_name, id)

    def to_module(self):
        return f"op_HLS_DEC_LOAD_{self.depth}_{self.type_name}_I{self.addr_width}W_I{self.width}W_O{self.width}W_O{self.addr_width}W"

    def to_instance(self):
        return f"op_HLS_DEC_LOAD_{self.depth}_{self.type_name}_IN2_W{self.width}_OUT2_W{self.addr_width}_{self.id}"
def find_sr_path(hier):
    for name, h in hier.items():
        if name == "sr":
            return (name, )
        if path := find_sr_path(h):
            return (name,)+path

def to_pow2(x):
    if x:
        return int(2**math.ceil(math.log2(x)))
    return 0

def find_op_names(vcd: vcdlib.Vcd, name_contains: str):
    sr_path = find_sr_path(vcd.net_hier)
    hier = vcd.net_hier
    for n in sr_path:
        hier = hier[n]
    result = []
    for name in hier:
        if name_contains in name and "ready" not in name and "valid" not in name and "data" not in name:
            result.append(name)
    return result

@dataclasses.dataclass
class RhlsAccelerator:
    verilog_path: pathlib.Path
    @property
    def vcd_path(self):
        return self.verilog_path.with_suffix(".vcd")
    @property
    def dot_path(self):
        return self.verilog_path.with_suffix(".dot")
    @property
    def cpp_path(self):
        return self.verilog_path.with_suffix(".harness.cpp")
    @property
    def object_path(self):
        return self.verilog_path.with_suffix(".o")
    @property
    def json_path(self):
        return self.verilog_path.with_suffix(".json")

    @property
    def sim_path(self):
        return self.verilog_path.with_suffix("")

    def build_sim(self, mem_latency: int = 100):
        HLS_TEST_ROOT = pathlib.Path("..").absolute()
        with tempfile.TemporaryDirectory() as tmpdir:
            subprocess.run(f"""verilator --trace -CFLAGS -DTRACE_SIGNALS --cc --build --exe -Wno-WIDTH -j 12 -y {HLS_TEST_ROOT}/verilog_ops/float -y {HLS_TEST_ROOT}/verilog_ops/float/dpi -y {HLS_TEST_ROOT}/verilog_ops/buffer -y {HLS_TEST_ROOT}/verilog_ops/dec_load -Mdir {tmpdir}  -MAKEFLAGS CXX=g++ -CFLAGS -g --assert -CFLAGS " -fPIC" -o {self.sim_path.absolute()} {self.verilog_path.absolute()} {self.object_path.absolute()} {self.cpp_path.absolute()} {HLS_TEST_ROOT}/verilog_ops/float/dpi/float_dpi.cpp {HLS_TEST_ROOT}/src/decoupled/c_sim/hls_stream.cpp {HLS_TEST_ROOT}/src/decoupled/c_sim/hls_decouple.cpp -CFLAGS -DMEMORY_LATENCY={mem_latency}""", shell=True, check=True)
            assert self.sim_path.exists()
        return self.sim_path

    def run_sim(self):
        assert self.sim_path.exists()
        out = subprocess.run(self.sim_path, capture_output = True, text = True)
        assert out.returncode == 0
        cycles = re.findall("finished - took ([0-9]+) cycles", out.stdout)
        assert len(cycles) == 1
        cycles = int(cycles[0])
        vcd = pathlib.Path(f"V{self.sim_path.stem}.vcd")
        new_vcd = self.sim_path.with_suffix(".hls.vcd")
        assert vcd.exists()
        vcd.replace(new_vcd)
        return cycles

    def upsize_buffers(self, buffer_size: int=256):
        verilog = self.verilog_path.read_text()
        dot = self.dot_path.read_text()
        buffer_matches = sorted(re.findall(r'((op_HLS_BUF_[^\s]+)\s+(op_HLS_BUF_[^\s]+))\s*\(', verilog))
        for instantiation, module, instance in buffer_matches:
            nrb, dot, verilog = self._resize_buf(instance, buffer_size, dot, verilog)
        load_matches = sorted(re.findall(r'((op_HLS_DEC_LOAD_[^\s]+)\s+(op_HLS_DEC_LOAD_[^\s]+))\s*\(', verilog))
        for instantiation, module, instance in load_matches:
            nrl, dot, verilog = self._resize_load(instance, buffer_size, dot, verilog)
        cpp = self.cpp_path.read_text().replace('".vcd"', '".upsize.vcd"')
        new = RhlsAccelerator(self.verilog_path.with_name(self.verilog_path.name.replace(".hls", ".upsize.hls")))
        new.dot_path.write_text(dot)
        new.verilog_path.write_text(verilog)
        new.cpp_path.write_text(cpp)
        shutil.copy(self.object_path, new.object_path)
        shutil.copy(self.json_path, new.json_path)
        return new

    def resize_buffers(self):
        vcd = vcdlib.Vcd(self.vcd_path)
        sr_path = find_sr_path(vcd.net_hier)
        hier = vcd.net_hier
        for p in sr_path:
            hier = hier[p]
        bufs = find_op_names(vcd, "op_HLS_BUF")
        buf_depths = {}
        for buf in bufs:
            # this requires using chisel bufs with a queue that exposes a count
            count_path = sr_path + (buf, "queue", "io_count")
            count_vcd = vcd[count_path]
            depth = max((int(val, 2) for ts, val in count_vcd.time_series if ts > vcd.start_time), default=0)
            buf_depths[buf] = depth
            # empty_path = sr_path + (buf, "queue", "empty")
            # empty_vcd = vcd[empty_path]
            # time_steps = 0
            # empty = 0
            # for ts in range(vcd.start_time, vcd.last_change, vcd.time_step*2):
            #     time_steps += 1
            #     if(empty_vcd[ts]):
            #         empty += 1
            # # if depth == 0:
            #     # assert empty == time_steps
            # empty_frac = empty/time_steps
        pprint.pprint(buf_depths)
        loads = find_op_names(vcd, "op_HLS_DEC_LOAD")
        load_depths = {}
        for load in loads:
            # this requires using chisel bufs with a queue that exposes a count
            count_path = sr_path + (load, "request_in_flight", "io_count")
            count_vcd = vcd[count_path]
            depth = max((int(val, 2) for ts, val in count_vcd.time_series if ts > vcd.start_time), default=0)
            load_depths[load] = depth
        pprint.pprint(load_depths)
        verilog = self.verilog_path.read_text()
        dot = self.dot_path.read_text()
        needed_ops = []
        for buf, depth in buf_depths.items():
            rb = RhlsBuf.from_instance(buf)
            if depth < 2:
                if rb.pass_through:
                    # empty or small pass-through buffers are fine
                    pass
                else:
                    # needed to guarantee forward progress
                    depth = 2
            depth = to_pow2(depth)
            nrb, dot, verilog = self._resize_buf(buf, depth, dot, verilog)
            needed_ops.append(nrb.to_chisel())
        for load, depth in load_depths.items():
            depth = to_pow2(depth)
            nrl, dot, verilog = self._resize_load(load, depth, dot, verilog)
            needed_ops.append(nrl.to_chisel())
        pprint.pprint(collections.Counter(needed_ops))
        for n in sorted(set(needed_ops)):
            print(n)
        cpp = self.cpp_path.read_text()
        if ".upsize" in self.vcd_path.name:
            new = RhlsAccelerator(self.verilog_path.with_name(self.verilog_path.name.replace(".upsize.hls", ".resize.hls")))
            cpp = cpp.replace('".upsize.vcd"', '".resize.vcd"')
        else:
            new = RhlsAccelerator(self.verilog_path.with_name(self.verilog_path.name.replace(".hls", ".resize.hls")))
            cpp = cpp.replace('".vcd"', '".resize.vcd"')
        new.dot_path.write_text(dot)
        new.verilog_path.write_text(verilog)
        new.cpp_path.write_text(cpp)
        shutil.copy(self.object_path, new.object_path)
        shutil.copy(self.json_path, new.json_path)
        return new

    @classmethod
    def _resize_buf(self, buf: str, depth: int, dot: str, verilog: str):
        rb = RhlsBuf.from_instance(buf)
        nrb = dataclasses.replace(rb, depth=depth)
        old_instantiation = rb.to_module() + " " + rb.to_instance() + " "
        new_instantiation = nrb.to_module() + " " + nrb.to_instance() + " "
        assert old_instantiation in verilog
        verilog = verilog.replace(old_instantiation, new_instantiation)
        verilog = verilog.replace(rb.to_instance() + ".", nrb.to_instance() + ".")
        dot = dot.replace(rb.to_instance() + " ", nrb.to_instance() + " ")
        dot = dot.replace(rb.to_instance() + "<", nrb.to_instance() + "<")
        dot = dot.replace(rb.to_instance() + ":", nrb.to_instance() + ":")
        return nrb, dot, verilog

    @classmethod
    def _resize_load(self, load: str, depth: int, dot: str, verilog: str):
        rl = RhlsDecLoad.from_instance(load)
        nrl = dataclasses.replace(rl, depth=depth)
        old_instantiation = rl.to_module() + " " + rl.to_instance() + " "
        new_instantiation = nrl.to_module() + " " + nrl.to_instance() + " "
        assert old_instantiation in verilog
        verilog = verilog.replace(old_instantiation, new_instantiation)
        verilog = verilog.replace(rl.to_instance()+".", nrl.to_instance()+".")
        dot = dot.replace(rl.to_instance()+" ", nrl.to_instance()+" ")
        dot = dot.replace(rl.to_instance()+"<", nrl.to_instance()+"<")
        dot = dot.replace(rl.to_instance()+":", nrl.to_instance()+":")
        return nrl, dot, verilog


build_folder = pathlib.Path("../build")

def main():
    verilogs = list(build_folder.glob("*/*.hls.v"))
    # verilogs = [build_folder / "decoupled" / "test_sum.hls.v"]
    for verilog in verilogs:
        if ".upsize" in verilog.name:
            continue
        acc = RhlsAccelerator(verilog)
        acc.build_sim()
        cycles = acc.run_sim()
        up_acc = acc.upsize_buffers()
        up_acc.build_sim()
        up_cycles = up_acc.run_sim()
        assert up_cycles <= cycles
        re_acc = up_acc.resize_buffers()
        re_acc.build_sim()
        re_cycles = re_acc.run_sim()
        assert re_cycles == up_cycles
        pass



if __name__ == '__main__':
    main()