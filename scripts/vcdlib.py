import Verilog_VCD.Verilog_VCD as vvcd


class Vcd:
    def __init__(self, path):
        data = vvcd.parse_vcd(path)
        self.net_dict = {}
        self.last_time = max(r['tv'][-1][0] for r in data.values())
        last_change = 0
        start_time = 0
        self.net_hier = {}
        self.time_step = 1e9
        for record in data.values():
            tv = record["tv"]
            self.time_step = min(self.time_step, min((tv[i+1][0]-tv[i][0] for i in range(len(tv)-1)), default=self.time_step))
            ts = VcdTimeSeries(tv)
            hiers = set()
            for net in record["nets"]:
                hier = tuple(net["hier"].split(".")) + (net["name"].split('[')[0],)
                hiers.add(hier)
                self.net_dict[hier] = ts
                d = self.net_hier
                for h in hier:
                    d = d.setdefault(h, {})
            if ("TOP", "clk") not in hiers:
                last_change = max(ts.last_time, last_change)
            if ("TOP", "reset") in hiers:  # assumes there is only one reset
                start_time = ts.last_time
        self.start_time = start_time
        self.last_change = last_change

    def __getitem__(self, hier):
        return self.net_dict[hier]

    def __contains__(self, hier):
        return hier in self.net_dict

    def get(self, hier, default):
        return self.net_dict.get(hier, default)


class VcdTimeSeries:
    def __init__(self, ts):
        self.time_series = ts
        # create a per-instance cache
        self.cache = {}

    def __getitem__(self, ts):
        if ts in self.cache:
            return self.cache[ts]
        l, r = 0, len(self.time_series) - 1
        while r - l >= 2:
            m = (r + l) // 2
            if self.time_series[m][0] > ts:
                r = m
            else:
                l = m
        assert self.time_series[l][0] <= ts
        if self.time_series[r][0] <= ts:
            result = self.time_series[r][1]
        else:
            result = self.time_series[l][1]
        result = int(result, 2)
        self.cache[ts] = result
        return result

    @property
    def last_time(self):
        return self.time_series[-1][0]