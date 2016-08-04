#!/usr/bin/env python3

import optparse

class RcpLogValidator(object):
    MAX_ERRORS = 25
    MAX_INTERVAL_STEP_MS = 3000
    MIN_INTERVAL_STEP_MS = 1

    def __init__(self):
        self.interval = 0
        self.columns = 0

    def _count_commas(self, line):
        return line.count(',')

    def _validate_column_count(self, line, line_no):
        columns = self._count_commas(line)
        if columns  == self.columns:
            return True

        # If here we failed. Print msg.
        print(("Error: Column count mismatch on line {}. Expected {}, " +
               "found {}").format(line_no, self.columns, columns))

        return False

    def _validate_interval_time(self, line, line_no):
        try:
            interval = int(line.split(',', 1)[0])
        except ValueError:
            interval = 0

        # Deal with initialization.
        if self.interval == 0:
            self.interval = interval
            return True

        interval_min = self.interval + self.MIN_INTERVAL_STEP_MS
        interval_max = self.interval + self.MAX_INTERVAL_STEP_MS

        if interval_min <= interval and interval <= interval_max:
            self.interval = interval
            return True

        # If here we failed. Print msg.
        print(("Error: Inconsistent interval value on line {}. Expected " +
               "{} - {}, found {}").format(line_no, interval_min, \
                                           interval_max, interval))
        return False

    def _validate_line(self, line, line_no='?'):
        # Check if its a header line.
        if '|' in line:
            self.columns = self._count_commas(line)
            return True

        return \
            self._validate_column_count(line, line_no) and \
            self._validate_interval_time(line, line_no)


    def validate_logfile(self, log_path):
        errors = 0
        line_no = 0

        with open(log_path, 'r') as fil:
            for line in fil:
                line_no += 1

                if errors >= self.MAX_ERRORS:
                    break

                if not self._validate_line(line, line_no):
                    errors += 1

        print()
        print("=== Final Stats ===")
        print("   Lines Checked: {}".format(line_no))
        print("   Errors Found:  {}".format(errors))
        print()

    def clean_logfile(self, input_path, output_path):
        errors = 0
        line_no = 0

        with open(input_path, 'r') as file_in:
            with open(output_path, 'w') as file_out:
                for line in file_in:
                    line_no += 1

                    if not self._validate_line(line, line_no):
                        errors += 1
                    else:
                        file_out.write(line)

        print()
        print("=== Processing Stats ===")
        print("   Lines Checked: {}".format(line_no))
        print("   Lines Pruned:  {}".format(errors))
        print()


def main():
    parser = optparse.OptionParser()
    parser.add_option('-f', '--filename',
                      dest="log_file",
                      help="Path of log file to process")

    parser.add_option('-o', '--output',
                      dest="out_file",
                      help="Path to output file of processed data")

    options, remainder = parser.parse_args()

    if not options.log_file:
        parser.error("No log file path given")

    rcplv = RcpLogValidator()
    if not options.out_file:
        rcplv.validate_logfile(options.log_file)
    else:
        rcplv.clean_logfile(options.log_file, options.out_file)

if __name__ == '__main__':
    main()
