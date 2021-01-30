import subprocess
import os
import sys

# Configuration:
INDENT_CHR = '    '
MAX_DEPTH = 0

exit_code = 0

class TestResult:
    def __init__(self, name, path):
        self.name = name
        self.path = path
        self.level = 0

    def indent(self, m = 0):
        print(("|" + INDENT_CHR) * m, end='')
        print(("|" + INDENT_CHR) * self.level, end='')

class DirectoryTestResult(TestResult):
    def __init__(self, name, path):
        super().__init__(name, path)
        self.tests = []
        self.modules = []

        self.successes = 0
        self.fails = []

    def add_result(self, t):
        self.tests.append(t)
        t.level = self.level + 1
        if t.success:
            self.successes += 1
        else:
            self.fails.append(t)

    def add_module(self, m):
        self.modules.append(m)
        m.level = self.level + 1
        m.update_levels()

    def update_levels(self):
        for m in self.modules:
            m.level = self.level + 1
            m.update_levels()

        for t in self.tests:
            t.level = self.level + 1

    def total_count(self):
        successes = self.successes
        fails = len(self.fails)

        nested_modules = len(self.modules)

        for x in self.modules:
            m_count = x.total_count()
            successes += m_count[0]
            fails += m_count[1]

            nested_modules += m_count[2]

        return (successes, fails, nested_modules)

    def get_fails(self, nested=True):
        fails = [(self.name, self.fails)]

        if nested:
            for m in self.modules:
                fails += m.get_fails()

        return fails

    def show(self):
        self.indent()
        print(f"Module {self.name}:")

        if self.level >= MAX_DEPTH:
            cnt = self.total_count()
            fails_found = self.get_fails()
            self.indent(1)
            print(f"{cnt[2]} nested modules hidden")

            self.indent(1)
            if cnt[1]:
                print(f"{cnt[0]}/{cnt[0]+cnt[1]} tests passed")
                for x in fails_found:
                    if not x[1]: continue
                    self.indent(1)
                    print()
                    self.indent(1)
                    print(f"fails in {x[0]}:")
                    for f in x[1]:
                        self.indent(2)
                        print(f.name + ":", f.path)
            else:
                print(f"All {cnt[0]}/{cnt[0]+cnt[1]} test passed!")
        else:
            if self.tests:
                self.indent(1)
                print(f"{self.successes} tests passed")
                self.indent(1)
                print(f"{len(self.fails)} tests failed")

            if self.fails:
                self.indent(1)
                print()
                self.indent(1)
                print("The tests that failed are:")
                for x in self.fails:
                    self.indent(2)
                    print(x.name + ":", x.path)

            for x in self.modules:
                self.indent(1)
                print()
                x.show()


class FileTestResult(TestResult):
    def __init__(self, name, path, good):
        super().__init__(name, path)
        self.success = good
        if not good:
            global exit_code
            exit_code = 1

    def total_count(self):
        return (1, 0, 0) if self.success else (0, 1, 0)


def test_file(path):
    try:
        program = subprocess.run(["odo", path], capture_output=True, text=True, check=True)
        if program.stdout == "bad": return (1, "bad")
        if program.stdout == "good": return (0, "good")

        return (2, "bad_test_output")
    except:
        return (1, "bad")


def test_directory(path, name=None):
    name = name or os.path.basename(path)[:-2]
    dir_results = DirectoryTestResult(name, path)
    files = os.listdir(path)

    for f in files:
        fullpath = os.path.join(path, f)
        if os.path.isfile(fullpath):
            if f.endswith('.todo'):
                file_name = f[:-5]
                f_result = test_file(fullpath)

                was_successful = f_result[0] == 0 and f_result[1] == "good"
                if file_name.endswith('_error'):
                    was_successful = not was_successful

                file_test_result = FileTestResult(file_name, fullpath, was_successful)
                dir_results.add_result(file_test_result)
        elif f.endswith('_m'):
            f_result = test_directory(fullpath)
            dir_results.add_module(f_result)

    return dir_results

print("\nRunning tests!")

a = test_directory('.', 'all')
a.show()

print()

sys.exit(exit_code)