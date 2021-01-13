import os
import enum

from typing import List


class ProcessingType(enum.Enum):
    COMPLETE = enum.auto()
    UTILITY = enum.auto()
    REPLACEMENT = enum.auto()


PROCESSING_TYPE = ProcessingType.REPLACEMENT


def main():
    # Specify folder to perform wrappin on
    read_all_files("..", True)


def read_all_files(folder, root_folder: bool, nesting_depth: int = 0):
    for file in os.listdir(folder):
        # Add files to be ignored
        if file in [".git", ".idea", ".project", ".settings"]:
            continue
        if ".py" in file or ".a" in file:
            continue
        # Replace fsfw by the folders which should be searched
        if file in ["doc", "etl", "_obj", "_dep", "_bin"] and root_folder:
            continue
        if os.path.isdir(os.path.join(folder, file)):
            read_all_files(os.path.join(folder, file), False, nesting_depth + 1)
            continue
        print("Checking file " + str(os.path.join(folder, file)))
        with open(os.path.join(folder, file), "r") as in_file:
            try:
                rows = [x for x in in_file]
            except UnicodeDecodeError:
                print("Decoding error in file " + str(file) + ", continuing..")
                continue
            do_write, rows = process_rows(rows)
        if do_write:
            with open(os.path.join(folder, file), "w") as in_file:
                print("Write file %s" % (os.path.join(folder, file)))
                in_file.writelines(rows)


def process_rows(rows: List[str]):
    multi_line = False
    write = False
    for index, row in enumerate(rows):
        if PROCESSING_TYPE == ProcessingType.COMPLETE:
            new_lines = row
            if "sif::" in row:
                print("SIF found in line " + str(index) + " file " + str(file))
                define_line = "#if FSFW_CPP_OSTREAM_ENABLED == 1\n"
                new_lines = define_line + new_lines
                if "std::endl" in row or "std::flush" in row:
                    print("One line SIF output from line" + str(index))
                    new_lines += "#endif\n"
                    write = True
                elif not multi_line:
                    multi_line = True
            if multi_line:
                if "std::endl" in row or "std::flush" in row:
                    print("Multiline output found in " + str(file) + " with ending on row "
                          + str(index))
                    new_lines += "#endif\n"
                    write = True
                    multi_line = False
            rows[index] = new_lines
        elif PROCESSING_TYPE == ProcessingType.UTILITY:
            new_line = row
            if "CPP_OSTREAM_ENABLED == 1" in row:
                new_line = row.replace("CPP_OSTREAM_ENABLED == 1", "FSFW_CPP_OSTREAM_ENABLED == 1")
                write = True
            rows[index] = new_line
        elif PROCESSING_TYPE == ProcessingType.REPLACEMENT:
            new_line = row
            if "fsfw::print" in row:
                new_line = row.replace("fsfw::print", "sif::print")
                write = True
            if "fsfw::Output" in row:
                new_line = row.replace("fsfw::Output", "sif::Output")
                write = True
            if "fsfw::ANSI" in row:
                new_line = row.replace("fsfw::ANSI", "sif::ANSI")
                write = True
            rows[index] = new_line
    return write, rows


if __name__ == "__main__":
    main()
