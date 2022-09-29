#!/usr/bin/env python3

"""TODO"""


import numpy as np
import warnings
import os
import pathlib
import shutil
import datetime
import subprocess
import traceback
#import sys
#import glob
import json
import yaml
import copy
import pickle
import scipy
import scipy.spatial



def tee(log_filename, *args):
    with open(log_filename, "a") as f:
       print(*args, file=f) 
    print(*args)


def interindiv_dist(xs, ys):
    d = np.zeros(xs.shape[0])
    for k, (x, y) in enumerate(zip(xs, ys)):
        for xi, yi in zip(x, y):
            for xj, yj in zip(x, y):
                d[k] += scipy.spatial.distance.euclidean([xi, yi], [xj, yj])
    return d / (x.shape[0] * x.shape[0])

def mean_neighbors_dist(xs, ys, fop=85):
    d = np.zeros(xs.shape[0])
    for k, (x, y) in enumerate(zip(xs, ys)):
        n = 0.
        for xi, yi in zip(x, y):
            for xj, yj in zip(x, y):
                eucl = scipy.spatial.distance.euclidean([xi, yi], [xj, yj])
                if eucl <= fop:
                    d[k] += eucl
                    n += 1.
        d[k] /= n
    return d

ref_disk_xy = np.array(
       [[136.10451306,   0.67873303],
        [203.08788599,   0.        ],
        [267.93349169,  33.93665158],
        [300.        ,  96.3800905 ],
        [167.4584323 , 164.25339367],
        [285.03562945, 232.12669683],
        [236.57957245, 278.95927602],
        [163.89548694, 300.        ],
        [ 99.76247031, 286.42533937],
        [ 40.6175772 , 253.84615385],
        [  0.        , 192.08144796],
        [  8.55106888, 125.56561086],
        [ 35.62945368,  62.44343891],
        [ 74.82185273,  20.36199095],
        [121.85273159,  52.94117647],
        [ 74.82185273, 111.99095023],
        [ 71.25890736, 183.93665158],
        [122.56532067, 230.09049774],
        [187.41092637, 229.41176471],
        [245.13064133, 190.04524887],
        [248.6935867 , 115.38461538],
        [199.52494062,  71.26696833],
        [155.34441805, 113.34841629],
        [124.70308789, 169.00452489],
        [196.67458432, 157.46606335]])

ref_annulus_xy = np.array(
       [[166.30824373,   0.        ],
        [101.7921147 ,  16.84981685],
        [ 96.05734767,  82.05128205],
        [ 40.14336918,  77.65567766],
        [ 56.63082437, 141.39194139],
        [  0.        , 168.4981685 ],
        [ 38.70967742, 221.97802198],
        [  0.        , 269.5970696 ],
        [ 65.94982079, 291.57509158],
        [ 55.91397849, 346.52014652],
        [117.56272401, 339.19413919],
        [141.93548387, 400.        ],
        [192.11469534, 359.70695971],
        [237.99283154, 397.8021978 ],
        [278.13620072, 348.71794872],
        [341.21863799, 359.70695971],
        [334.76702509, 290.84249084],
        [396.41577061, 264.46886447],
        [357.70609319, 213.18681319],
        [400.        , 158.97435897],
        [341.93548387, 136.26373626],
        [345.51971326,  57.87545788],
        [285.3046595 ,  68.86446886],
        [266.66666667,   2.93040293],
        [211.46953405,  42.49084249]])


ref_worse_disk_xy = np.array(
       [[-110.46955402,  100.41155655],
        [ 148.60510575,   13.89087915],
        [  78.62973852,  126.88353916],
        [  74.98596944, -129.11366067],
        [-146.42594052,  -28.9868404 ],
        [ -91.41409443, -117.97905877],
        [ 114.53987504,  -95.77798274],
        [ -21.14972148,  147.76693932],
        [  18.64707577, -148.11967566],
        [  34.        ,  -36.        ],
        [ 141.38345752,   47.8242569 ],
        [ 147.85911295,  -20.33358759],
        [  47.70646122,  141.45096115],
        [ 139.4066529 ,  -53.31728266],
        [ 105.68511635,  105.41232995],
        [-135.84609917,  -61.85748902],
        [-116.89757453,  -92.7927265 ],
        [  13.70938601,  148.64619304],
        [ -84.30104314,  123.21031537],
        [-131.02557639,   71.50577589],
        [-149.17578075,    5.08453861],
        [ -54.19215348,  139.09556856],
        [-143.94376333,   39.50722883],
        [-110.28359526,   31.41317228],
        [ 126.80520588,   78.73211235]])

ref_worse_annulus_xy = np.array(
       [[ 138.78446473, -137.15593336],
        [-184.81340461,  -62.97938804],
        [  40.62455989, -190.75645379],
        [ -49.49737522,  188.8614378 ],
        [ -59.        , -113.        ],
        [-347.86742371, -375.30030435],
        [-163.79837154,   24.96247001],
        [-188.8605933 ,   49.57668725],
        [-335.86742371, -375.30030435],
        [ 156.0824134 ,  117.24837727],
        [-343.86742371, -375.30030435],
        [ 160.93531964, -110.2251269 ],
        [-339.86742371, -375.30030435],
        [ 131.        ,  -55.        ],
        [-139.6733181 ,  136.64822997],
        [ 190.77240914,  -41.70679208],
        [ -62.16610132, -185.04807851],
        [ 174.06106955,   87.92041805],
        [ -82.42439277,  177.17094905],
        [-161.72729103,  109.82603824],
        [ -28.39509641, -193.10852886],
        [   6.83919137, -194.98671628],
        [-113.36423439,  159.07194735],
        [ 117.        ,  -14.        ],
        [ 190.92671169,   41.70343806]])



def occupied_surface(xs, ys, fop):
    return 1.0 # TODO



class KilomboLauncher:
    def __init__(self, exec_path = "dispersion", base_dir = "."):
        self.exec_path = os.path.abspath(exec_path)
        self.base_dir = base_dir

    def create_config(self, config_path, config):
        with open(config_path, "w") as f:
            f.write(json.dumps(config))
            f.flush()
            os.fsync(f)

    def analyse_kilombo_results(self, data_path):
        with open(data_path, "r") as f:
            raw_data = f.read()
        data = json.loads(raw_data)
        nb_entries = len(data)
        self.max_ticks = data[-1]["ticks"]
        self.nb_bots = len(data[0]["bot_states"])

        def get_state(x, key):
            return [s["state"][key] if key in s["state"] else np.nan for s in x["bot_states"]]

        def get_state2(x, b, key):
            state = data[x]["bot_states"][b]["state"]
            return state[key] if key in state else np.nan

        def get_vals(x, key):
            return [s[key] for s in x["bot_states"]]

        # X,Y positions
        self.x_position = np.array([get_vals(data[x], "x_position") for x in range(nb_entries)])
        self.y_position = np.array([get_vals(data[x], "y_position") for x in range(nb_entries)])

        res = {}
        res["x_position"] = self.x_position
        res["y_position"] = self.y_position
        return res


    def _run_kilombo(self, instance_base_path, config_path, log_path):
        cmd = [self.exec_path, "-p", config_path]
        try:
            output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=instance_base_path)
        except Exception as e:
            warnings.warn("ERROR during Kilombo execution with command: %s" % str(cmd), RuntimeWarning)
            traceback.print_exc()
            output = None
        try:
            with open(os.path.join(instance_base_path, log_path), "wb") as f:
                f.write(output)
        except Exception as e:
            warnings.warn("ERROR saving Kilombo logs with command: %s" % str(cmd), RuntimeWarning)
            traceback.print_exc()
        return output is not None

    def _clean_dir(self, instance_base_path):
        shutil.rmtree(pathlib.Path(instance_base_path))

    def launch(self, config, keep_tmp_files = False, keep_bugged_files = False):
        # Init file paths
        config_hash = hash(frozenset(config.items()))
        instance_name = datetime.datetime.now().strftime('%Y%m%d%H%M%S') + "_" + str(abs(config_hash)) + "_" + str(config['randSeed'])
        instance_base_path = os.path.join(self.base_dir, instance_name)
        # Create instance directory
        pathlib.Path(instance_base_path).mkdir(parents=True, exist_ok=True)
        # Set kilombo files path
        config_path = "kilombo.json"
        log_path = "log.txt"
        data_path = os.path.join(instance_base_path, "data.json")

        # Create kilombo config file
        config['stateFileName'] = os.path.abspath(data_path)
        config['GUI'] = 0
        config['arenaFileName'] = os.path.abspath(config['arenaFileName'])

        self.create_config(os.path.join(instance_base_path, config_path), config)

        # Run kilombo and get result data
        exec_success = self._run_kilombo(instance_base_path, config_path, log_path)
        if not exec_success:
            if not keep_bugged_files and not keep_tmp_files:
                self._clean_dir(instance_base_path)
            return None

        # Analyse the results from kilombo
        res = self.analyse_kilombo_results(data_path)
        #print(f"# {instance_name}: {res}")

        # Delete temporary files
        if not keep_tmp_files:
            self._clean_dir(instance_base_path)
        return res


def launch_kilombo(base_config, arenaFileName, seed, launcher, args):
    config = copy.deepcopy(base_config)
    config['randSeed'] = seed
    config['arenaFileName'] = arenaFileName
    instance_res = launcher.launch(config,
            keep_tmp_files = base_config.get('keep_tmp_files', False), keep_bugged_files = base_config.get('keep_bugged_files', False))
    return instance_res



def compute_stats_per_arena(data, config, output_path, log_filename):
    all_stats = {}
    # Stats related to positions of the robots
    all_stats['interindiv_dist'] = np.array([ interindiv_dist(r['x_position'], r['y_position']) for r in data])
    all_stats['mean_neighbors_dist'] = np.array([ mean_neighbors_dist(r['x_position'], r['y_position'], config['commsRadius']) for r in data])
    all_stats['occupied_surface'] = np.array([ occupied_surface(r['x_position'], r['y_position'], config['commsRadius']) for r in data])
    return all_stats


def compute_stats_all_arenas(data_per_arena, stats_per_arena, config, output_path):
    stats = {}

    if 'disk' in stats_per_arena:
        max_mnd_disk = mean_neighbors_dist(np.array([ref_disk_xy[:,0]]), np.array([ref_disk_xy[:,1]]), config['commsRadius'])[0]
        min_mnd_disk = mean_neighbors_dist(np.array([ref_worse_disk_xy[:,0]]), np.array([ref_worse_disk_xy[:,1]]), config['commsRadius'])[0] - 2.0
        mnd_disk = (stats_per_arena['disk']['mean_neighbors_dist'][:,-1] - min_mnd_disk) / (max_mnd_disk - min_mnd_disk)
        stats['mean_mnd_disk'] = mnd_disk.mean()
        stats['std_mnd_disk'] = mnd_disk.std()
    else:
        stats['mean_mnd_disk'] = None
        stats['std_mnd_disk'] = None
    if 'annulus' in stats_per_arena:
        max_mnd_annulus = mean_neighbors_dist(np.array([ref_annulus_xy[:,0]]), np.array([ref_annulus_xy[:,1]]), config['commsRadius'])[0]
        min_mnd_annulus = mean_neighbors_dist(np.array([ref_worse_annulus_xy[:,0]]), np.array([ref_worse_annulus_xy[:,1]]), config['commsRadius'])[0] - 2.0
        mnd_annulus = (stats_per_arena['annulus']['mean_neighbors_dist'][:,-1] - min_mnd_annulus) / (max_mnd_annulus - min_mnd_annulus)
        stats['mean_mnd_annulus'] = mnd_annulus.mean()
        stats['std_mnd_annulus'] = mnd_annulus.std()
    else:
        stats['mean_mnd_annulus'] = None
        stats['std_mnd_annulus'] = None

    # Return all stats
    return stats




if __name__ == "__main__":
    from multiprocessing import Pool

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--inputPath', type=str, default='test.yaml', help = "Path of the agents state file")
    parser.add_argument('-o', '--outputPath', type=str, default="results", help = "Path of the resulting csv file")
    parser.add_argument('-e', '--execPath', type=str, default="dispersion", help = "Path of the kilombo executable")
    parser.add_argument('-n', '--nbRuns', type=int, default=0, help = "Number of runs")
    parser.add_argument('-N', '--maxProcesses', type=int, default=0, help = "Max number of processes, or 0")
    parser.add_argument('-s', '--seed', type=int, default=42, help = "Random seed")
    parser.add_argument('--keep-tmp-files', dest="keep_tmp_files", action="store_true", help = "Keep temporary files")
    parser.add_argument('--keep-bugged-files', dest="keep_bugged_files", action="store_true", help = "Keep bugged files")
    parser.add_argument('--extended-logs', dest="extended_logs", action="store_true", help = "Outputs extra infos in results files and plot them")
    args = parser.parse_args()

    # Init base paths
    config_base_filename = os.path.splitext(os.path.basename(args.inputPath))[0]
    output_path = os.path.join(args.outputPath, config_base_filename)
    pathlib.Path(output_path).mkdir(parents=True, exist_ok=True)

    # Create and empty out log file
    log_filename = os.path.join(output_path, "log.txt")
    with open(log_filename, "w") as f:
        pass

    # Load base config
    base_config = yaml.safe_load(open(args.inputPath))
    initial_seed = args.seed
    # Set arena file names
    arenaFileNames = base_config['arenaFileNames']
    del base_config['arenaFileNames']
    # Set nb_runs
    nb_runs = args.nbRuns if args.nbRuns > 0 else base_config.get('nb_runs', 1) 

    # Check if we need to save extended data/stats into the logs and make plots of these stats
    extended_logs = args.extended_logs or base_config.get('extended_logs', False)
    base_config['extended_logs'] = extended_logs
    # Check if tmp and bugged files should be kept
    base_config['keep_tmp_files'] = args.keep_tmp_files or base_config.get('keep_tmp_files', False)
    base_config['keep_bugged_files'] = args.keep_bugged_files or base_config.get('keep_bugged_files', False)

    tee(log_filename, f"config_filename: {args.inputPath}")
    tee(log_filename, f"seed: {initial_seed}")
    tee(log_filename, f"arenaFileNames: {arenaFileNames}")
    tee(log_filename, f"nb_runs: {nb_runs}")
    tee(log_filename, f"maxProcesses: {args.maxProcesses}")

    pool_args = {} if args.maxProcesses <= 0 else {'processes': args.maxProcesses}
    seed = initial_seed
    data_per_arena = {}
    stats_per_arena = {}
    with Pool(**pool_args) as pool:
        for arenaFileName in arenaFileNames:
            tee(log_filename, f"\n### {arenaFileName} ###")
            expe_name = os.path.splitext(os.path.basename(arenaFileName))[0]
            output_path_full = os.path.join(output_path, expe_name)
            launcher = KilomboLauncher(base_dir = output_path_full, exec_path=args.execPath)
            params = [(base_config, arenaFileName, s, launcher, args) for s in range(seed, seed+nb_runs)]
            seed += nb_runs
            data_per_arena[expe_name] = pool.starmap(launch_kilombo, params)
            stats_per_arena[expe_name] = compute_stats_per_arena(data_per_arena[expe_name], base_config, output_path_full, log_filename)

    stats = compute_stats_all_arenas(data_per_arena, stats_per_arena, base_config, output_path)
    tee(log_filename, f"\n##############\n")
    tee(log_filename, "Final stats: ", stats)

    # Remove extended data/infos/stats if wanted
    if not extended_logs:
        for expe_name in data_per_arena.keys():
            data_per_arena[expe_name] = [{} for r in data_per_arena[expe_name]]

    # Save all results in a pickle file
    results = {'data_per_arena': data_per_arena, 'stats_per_arena': stats_per_arena, 'stats': stats}
    with open(os.path.join(output_path, "data.p"), "wb") as f:
        pickle.dump(results, f)



# MODELINE "{{{1
# vim:expandtab:softtabstop=4:shiftwidth=4:fileencoding=utf-8
# vim:foldmethod=marker
