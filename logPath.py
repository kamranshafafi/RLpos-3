import os
import datetime
import shutil  # Import the shutil module

def generate_log_directory(simType, dir):
    # Get the current timestamp in the format 'YYYYMMDDHHMMSS'
    now = datetime.datetime.now().strftime('%Y%m%d%H%M%S')

    # Use 'src_dir' as the working directory
    wd = os.getcwd()

    if simType == "eval":
        dirprefix = "e"
    elif simType == "train":
        dirprefix = "t"
    else:
        print("Error: Valid simTypes are 'eval' and 'train'. Not '{}'.".format(simType))
        exit(1)
    if not dir:
        rundir = os.path.join(wd, 'results', f'{dirprefix}{now}')
    else:
        rundir = os.path.join(wd, 'results', dir)

    # Check if 'rundir' directory exists, create it if it doesn't
    if not os.path.exists(rundir):
        os.makedirs(rundir)

    # Copy 'input.json' to 'rundir' (assuming it's in the same directory as the script)
    input_json = os.path.join(wd, 'config', 'input.json')
    if os.path.isfile(input_json):
        shutil.copy(input_json, rundir)

    return rundir