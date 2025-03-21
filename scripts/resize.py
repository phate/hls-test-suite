import collections
import dataclasses
import math
import pprint
import re

import vcdlib


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

def resize_buffers():
    vcd = vcdlib.Vcd("../build/decoupled/Vspmv_stream.vcd")
    sr_path = find_sr_path(vcd.net_hier)
    hier = vcd.net_hier
    for p in sr_path:
        hier = hier[p]
    bufs = find_op_names(vcd, "op_HLS_BUF")
    counts = {}
    for buf in bufs:
        # this requires using chisel bufs with a queue that exposes a count
        count_path = sr_path + (buf, "queue", "io_count")
        count_vcd = vcd[count_path]
        count = max((int(val, 2) for ts, val in count_vcd.time_series if ts > vcd.start_time), default=0)
        empty_path = sr_path + (buf, "queue", "empty")
        empty_vcd = vcd[empty_path]
        # time_steps = 0
        # empty = 0
        # for ts in range(vcd.start_time, vcd.last_change, vcd.time_step*2):
        #     time_steps += 1
        #     if(empty_vcd[ts]):
        #         empty += 1
        # # if count == 0:
        #     # assert empty == time_steps
        # empty_frac = empty/time_steps
        counts[buf] = count
    pprint.pprint(counts)
    with open("jlm_hls.v") as f:
        verilog = f.read()
    with open("jlm_hls.dot") as f:
        dot = f.read()
    needed_buffers = []
    for buf, count in counts.items():
        rb = RhlsBuf.from_instance(buf)
        if count < 2:
            count = 2
        # nrb = dataclasses.replace(rb, depth=count)
        nrb = dataclasses.replace(rb, depth=to_pow2(count))
        # nrb = dataclasses.replace(rb, depth=256)
        assert rb.to_instance() == buf
        old_instantiation = rb.to_module() + " " + rb.to_instance()
        new_instantiation = nrb.to_module() + " " + nrb.to_instance()
        assert old_instantiation in verilog
        verilog = verilog.replace(old_instantiation, new_instantiation)
        verilog = verilog.replace(rb.to_instance(), nrb.to_instance())
        dot = dot.replace(rb.to_instance(), nrb.to_instance())

        needed_buffers.append(nrb.to_chisel())
    pprint.pprint(collections.Counter(needed_buffers))
    for n in sorted(set(needed_buffers)):
        print(n)
    with open("jlm_hls_opt.v", "w") as f:
        f.write(verilog)
    with open("jlm_hls_opt.dot", "w") as f:
        f.write(dot)

if __name__ == '__main__':
    resize_buffers()