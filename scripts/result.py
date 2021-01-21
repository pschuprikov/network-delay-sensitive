import argparse
from json import dumps


class Bound:
    def __init__(self, lower, upper):
        self.lower = lower
        self.upper = upper

    @staticmethod
    def from_string(s: str):
        a, b = s.split(',')
        return Bound(int(a) if a else None, int(b) if b else None)

    def satisfies(self, value):
        if self.lower is not None and value < self.lower:
            return False
        if self.upper is not None and value > self.upper:
            return False
        return True


class FlowSetStats:
    def __init__(self, name: str, bound: Bound):
        self.bound = bound
        self.name = name
        self.deadlines = 0
        self.deadlines_met = 0
        self.nfcts = []
        self.fcts = []
        self.fcts_notimeout = []
        self.num_timeouts = 0
        self.gps = []
        self.non_deadline_gps = []

    def add(self, fct: float, size: int, deadline_is_met: bool, has_deadline: bool, num_timeouts: int):
        if not self.bound.satisfies(size):
            return

        # TODO: use idealized FCT?
        self.nfcts.append(fct / size)
        self.fcts.append(fct)
        self.gps.append(size * 8 / fct)
        if not has_deadline:
            self.non_deadline_gps.append(size * 8 / fct)
        if num_timeouts == 0:
            self.fcts_notimeout.append(fct)
        if deadline_is_met:
            self.deadlines_met += 1
        if has_deadline:
            self.deadlines += 1
        self.num_timeouts += num_timeouts

    @property
    def avg_gp(self) -> float:
        return sum(self.gps) / len(self.gps) if len(self.gps) > 0 else 0

    @property
    def avg_nondeadline_gp(self) -> float:
        return sum(self.non_deadline_gps) / len(self.non_deadline_gps) if len(self.non_deadline_gps) > 0 else 0

    @property
    def avg_fct(self) -> float:
        return sum(self.fcts) / len(self.fcts) if len(self.fcts) > 0 else float('inf')

    @property
    def avg_nfct(self) -> float:
        return sum(self.nfcts) / len(self.nfcts) if len(self.nfcts) > 0 else float('inf')

    @property
    def fct_99th(self) -> float:
        self.fcts.sort()
        if len(self.fcts) > 0:
            return self.fcts[99 * len(self.fcts) // 100]
        else:
            return 0

    @property
    def median_fct(self) -> float:
        self.fcts.sort()
        if len(self.fcts) > 0:
            return self.fcts[50 * len(self.fcts) // 100]
        else:
            return 0

    @property
    def num_flows(self) -> int:
        return len(self.fcts)

    @property
    def nfct_99th(self) -> float:
        self.nfcts.sort()
        if len(self.nfcts) > 0:
            return self.nfcts[99 * len(self.fcts) // 100]
        else:
            return 0


def report(report_avg, report_tail, report_num_deadlines, report_gp, report_avgn, json, stats: FlowSetStats):
    if json is not None and stats.name not in json:
        json[stats.name] = {}

    if report_avg:
        if json is None:
            print(f"Average FCT for {stats.num_flows} in {stats.name}:\n{stats.avg_fct}")
        else:
            json[stats.name]['avgfct'] = stats.avg_fct

    if report_gp:
        if json is None:
            print(f"Average Goodput for {stats.num_flows} in {stats.name}:\n{stats.avg_gp} (w/o slack: {stats.avg_nondeadline_gp}")
        else:
            json[stats.name]['gp'] = stats.avg_gp
            json[stats.name]['nd_gp'] = stats.avg_nondeadline_gp

    if report_tail:
        if json is None:
            print(f"99th percentile FCT for {stats.num_flows} in {stats.name}:\n{stats.fct_99th}")
        else:
            json[stats.name]['tailfct'] = stats.fct_99th

    if report_num_deadlines:
        if json is None:
            print(f'Num deadlines met for {stats.num_flows} in {stats.name}:\n{stats.deadlines_met}/{stats.deadlines}')
        else:
            json[stats.name]['thd'] = stats.deadlines_met
            json[stats.name]['max_thd'] = stats.deadlines
            json[stats.name]['thd_fraction'] = stats.deadlines_met / stats.deadlines if stats.deadlines != 0 else None

    if report_avgn:
        if json is None:
            print(f'Average normalized FCT for {stats.num_flows} in {stats.name}:\n{stats.avg_nfct}')
        else:
            json[stats.name]['avgnfct'] = stats.avg_nfct
            json[stats.name]['tailnfct'] = stats.nfct_99th


parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", help="input file name")
parser.add_argument("-b", "--bound", help="print all FCT information", action="append", type=Bound.from_string)
parser.add_argument("-a", "--all", help="print all FCT information", action="store_true")
parser.add_argument("-o", "--overall", help="print overall FCT information", action="store_true")
parser.add_argument("-s", "--small", help="print FCT information for short flows", action="store_true")
parser.add_argument("-m", "--median", help="print FCT information for median flows", action="store_true")
parser.add_argument("-l", "--large", help="print FCT information for large flows", action="store_true")
parser.add_argument("--avg",
                    help="print average FCT information for flows in specific ranges",
                    action="store_true")
parser.add_argument("--avgn",
                    help="pring average normalized FCT information for flows in specific ranges",
                    action="store_true")
parser.add_argument("--tail",
                    help="only print tail (99th percentile) FCT information for flows in specific ranges",
                    action="store_true")
parser.add_argument("--thd",
                    help="print number of deadlines met",
                    action="store_true")
parser.add_argument("--gp",
                    help="print goodput",
                    action='store_true')
parser.add_argument("--json", help="output JSON", action="store_true")

overall = FlowSetStats('overall', Bound(None, None))
short = FlowSetStats('short', Bound(None, 100 * 1024))
median = FlowSetStats('median', Bound(100 * 1024 + 1, 10 * 1024 * 1024))
large = FlowSetStats('large', Bound(10 * 1024 * 1024, None))


args = parser.parse_args()

custom_sets = [FlowSetStats(f'custom{i}', b) for i, b in enumerate(args.bound)] if args.bound else []
flowsets = [overall, short, median, large] + custom_sets

if args.input:
    fp = open(args.input)
    # Get overall average normalized FCT
    while True:
        line = fp.readline()
        if not line:
            break
        if len(line.split()) < 3:
            continue

        pkt_size, time, timeouts_num, _, _, deadline, *rest = line.split()

        pkt_size = int(float(pkt_size))
        byte_size = int(float(pkt_size) * 1460)
        time = float(time)
        timeouts_num = int(timeouts_num)
        deadline = float(deadline) * 1.e-6

        early_termination = int(rest[0]) == 1 if len(rest) == 1 else False
        deadline_is_met = deadline != 0.0 and time <= deadline and not early_termination
        has_deadline = deadline != 0.0 or early_termination

        if time == 0:
            continue


        for flow_set in flowsets:
            flow_set.add(time, byte_size, deadline_is_met, has_deadline, timeouts_num)

    fp.close()

    json = {} if args.json else None

    if args.all:
        print(f"There are {overall.num_flows} flows in total")
        print(f"There are {overall.num_timeouts} TCP timeouts in total")
        print(f"Overall average FCT is:\n{overall.avg_fct}")
        print(f"Average FCT for {short.num_flows} flows in (0,100KB)\n{short.avg_fct}")
        print(f"99th percentile FCT for {short.num_flows} flows in (0,100KB)\n{short.fct_99th}")
        print(f"Average FCT for {median.num_flows} flows in (100KB,10MB)\n{median.median_fct}")
        print(f"Average FCT for {large.num_flows} flows in (10MB,)\n{large.avg_fct}")

    # Overall FCT information
    if args.overall:
        report(args.avg, args.tail, args.thd, args.gp, args.avgn, json, overall)

    # FCT information for short flows
    if args.small:
        report(args.avg, args.tail, args.thd, args.gp, args.avgn, json, short)

    # FCT information for median flows
    if args.median:
        report(args.avg, args.tail, args.thd, args.gp, args.avgn, json, median)

    # FCT information for large flows
    if args.large:
        report(args.avg, args.tail, args.thd, args.gp, args.avgn, json, large)

    for custom_flow_set in custom_sets:
        report(args.avg, args.tail, args.thd, args.gp, args.avgn, json, custom_flow_set)

    if json is not None and not args.all:
        print(dumps(json))

