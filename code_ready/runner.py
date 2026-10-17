import os, csv, statistics, subprocess

from datetime import datetime
import matplotlib.pyplot as plt


def load_intel_environment():

    print("Loading Intel oneAPI environment...")
    
    cmd = "source /opt/intel/oneapi/setvars.sh > /dev/null 2>&1 && env"
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True, executable='/bin/bash')
    
    if res.returncode != 0:
        print("Warning: Failed to source Intel oneAPI setvars.sh")
        return

    for line in res.stdout.splitlines():
        if '=' in line:
            key, val = line.split('=', 1)
            os.environ[key] = val


def compile_version(folder, compiler_flag):
    
    src_file = folder + "/sobel_orig.c"
    exe_file = folder + "/sobel_orig"

    print(f"[{src_file}] [{compiler_flag}] compiling...")

    compile_cmd = f"icx -Wall {compiler_flag} {src_file} -o {exe_file}"
    
    compile_process = subprocess.run(compile_cmd, shell=True, capture_output=True, text=True)

    if compile_process.returncode != 0:
        print("COMPILATION ERROR")
        return False

    return exe_file


def get_time(output_str):

    for row in output_str.split('\n'):
        if "Total time =" in row:
            t = row.split()[-2]
            return float(t)

    print(f"Time did not found. output: {output_str}")
    return False


def run_version(exe_file):

    print(f"[{exe_file}] is running...")

    run_process = subprocess.run(f"./{exe_file}", shell=True, capture_output=True, text=True)
    return run_process


def check_diff():

    run_process = subprocess.run(f"diff output_sobel.grey golden.grey", shell=True, capture_output=True, text=True)
    if run_process.stdout:
        print("WRONG OUTPUT", run_process.stdout)


def calculate_stats(execution_times, version, compiler_flag):

    execution_times.sort()
    execution_times = execution_times[1:-1]
    
    mean_val = statistics.mean(execution_times)
    std_dev = statistics.stdev(execution_times)

    formatted_date = datetime.now().strftime("%d/%m/%Y %H:%M:%S")

    d = {
        "datetime": formatted_date,
        "version": version,
        "compiler_flag": compiler_flag,
        "mean_time_sec": f"{mean_val:.6f}",
        "std_dev_sec": f"{std_dev:.6f}",
        "total_valid_runs": len(execution_times)
    }

    for i in range(num_runs-2):
        d[f"run_{i + 1}"] = execution_times[i]

    return d


def write_results(csv_filename, new_results):

    existing_results = load_results_from_csv(csv_filename)

    results_dict = {}
    
    for row in existing_results:
        key = (row["version"], row["compiler_flag"])
        results_dict[key] = row
        
    for row in new_results:
        key = (row["version"], row["compiler_flag"])
        results_dict[key] = row

    with open(csv_filename, mode="w", newline="", encoding="utf-8") as csv_file:

        fieldnames = [
            "datetime",
            "version",
            "compiler_flag",
            "mean_time_sec",
            "std_dev_sec",
            "total_valid_runs"
        ]
        
        for i in range(num_runs - 2):
            fieldnames.append(f"run_{i + 1}")

        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        writer.writeheader()
            
        for key in results_dict.keys():
            writer.writerow(results_dict[key])


def load_results_from_csv(filename):
    results = []
    if not os.path.exists(filename):
        return results

    with open(filename, mode="r", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)
        for row in reader:
            results.append(row)
        
    return results


def plot_results(results):

    global optimization_flags
    
    x, y_o0, y_fast = [], [], []

    for row in results:

        if row["compiler_flag"] == optimization_flags[0]:
            x.append(row["version"])
            y_o0.append(float(row["mean_time_sec"]))

        elif row["compiler_flag"] == optimization_flags[1]:
            y_fast.append(float(row["mean_time_sec"]))

    plt.figure(figsize=(24, 12))
    
    plt.plot(x, y_o0, marker='o', label='-O0')
    plt.plot(x, y_fast, marker='s', label='-fast')

    plt.xlabel("Version")
    plt.ylabel("Mean Time (sec)")
    plt.title("Sobel Execution Time (-O0 vs -fast)")
    plt.xticks(rotation=45)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    plt.show()


def run_all(optimization_flags, versions):

    results = []

    for compiler_flag in optimization_flags:

        for version in versions:

            exe_file = compile_version(version, compiler_flag)
            if not exe_file:
                continue

            execution_times = []
            for i in range(num_runs):

                r = run_version(exe_file)
                if r.returncode != 0:
                    print(f"RUNTIME PROBLEM exitcode: {r.returncode}")
                t = get_time(r.stdout)

                if i == 0:
                    check_diff()

                execution_times.append(t)

            s = calculate_stats(execution_times, version, compiler_flag)

            results.append(s)

            print('\n')
        print('\n')

    return results


# ===================== SETTINGS start ===================== #

num_runs = 4
optimization_flags = ["-O0", "-O3 -ffast-math"]
csv_filename = "experimental_results.csv"
versions = [f"v_{i}" for i in range(14, 19)]

# ====================== SETTINGS end ====================== #
load_intel_environment()

results = run_all(optimization_flags, versions)
write_results(csv_filename, results)

results = load_results_from_csv(csv_filename)
plot_results(results)