import os
import sys
import subprocess


def list_subdirectories(directory):
    """
    Recursively list all subdirectories of the given directory that have at least one .o file.
    """
    subdirectories = []
    if not os.path.isdir(directory):
        return subdirectories

    has_o_files = any(f.endswith('.o') for f in os.listdir(directory))
    if has_o_files:
        subdirectories.append(directory)

    for item in os.listdir(directory):
        item_path = os.path.join(directory, item)
        if os.path.isdir(item_path):
            subdirectories += list_subdirectories(item_path)

    return subdirectories

# check if at least one directory is provided
if len(sys.argv) < 2:
    print('Usage: python script.py <argument 1> <directory 1> [directory 2] [directory 3] ...')
    sys.exit(1)

print(len(sys.argv))
# extract the first argument
arg1 = sys.argv[1]

# extract the list of directories
#directories = sys.argv[2:]

#directories = ["/home/roop/inner_unikernels/linux/arch/x86_64", "/home/roop/inner_unikernels/linux/fs", "/home/roop/inner_unikernels/linux/drivers", "/home/roop/inner_unikernels/linux/kernel/", "/home/roop/inner_unikernels/linux/mm/", "/home/roop/inner_unikernels/linux/net", "/home/roop/inner_unikernels/linux/usr"]

directories = ["/home/roop/inner_unikernels/linux/"]

# generate the list of subdirectories with .o files
subdirectories = []
for directory in directories:
    subdirectories += list_subdirectories(directory)

# construct the final string of .o files
o_files = ''
for directory in subdirectories:
    o_files += os.path.join(directory, '*.o') + ' '

# print the final command
print('lib/kanalyzer --dump-callgraph=1 --callgraph-entry={} {}'.format(arg1, o_files))
#subprocess.call(['lib/kanalyzer --dump-callgraph=1 --callgraph-entry={} {}'.format(arg1, o_files)])

