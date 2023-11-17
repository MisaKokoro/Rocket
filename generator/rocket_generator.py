import getopt
import traceback
import os
from string import Template

def parseInput():
    opts, args = getopt.getopt(sys.argv[1:], "hi:o", )

def generate_project():
    try:
        parseInput()

        print('=' * 150)
        print("Begin to generate rocket rpc server")

        generate_dir()

        generate_pb()
    except Exception as e:
        print("Failed to generate rocket server, err: " + e)
        trace.print_exc()
        print('=' * 150)

if __name__ == '__main__':
    generate_project()
