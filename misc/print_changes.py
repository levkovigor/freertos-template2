# Move this file into root folder of project !
import os


def main():
    read_all_files("..", True)


def read_all_files(folder, root_folder: bool, nesting_depth: int = 0):
    for file in os.listdir(folder):
        if file in [".git", ".idea", ".project", ".settings"]:
            continue
        if ".py" in file or ".a" in file:
            continue
        if file not in ["fsfw"] and root_folder:
            continue
        if os.path.isdir(os.path.join(folder, file)):
            read_all_files(os.path.join(folder, file), False, nesting_depth + 1)
            continue
        print("Checking file " + str(os.path.join(folder, file)))
        write = False
        with open(os.path.join(folder, file), "r") as in_file:
            try:
                rows = [x for x in in_file]
            except UnicodeDecodeError:
                print("Decoding error in file " + str(file) + ", continuing..")
                continue
            multi_line = False
            for index, row in enumerate(rows):
                new_lines = row
                if "sif::" in row:
                    print("SIF found in line " + str(index) + " file " + str(file))
                    define_line = "#ifdef CPP_OSTREAM_ENABLED\n"
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

        if write:
            with open(os.path.join(folder, file), "w") as in_file:
                print("Write file %s" % (os.path.join(folder, file)))
                in_file.writelines(rows)


if __name__ == "__main__":
    main()
