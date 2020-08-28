# Move this file into root folder of project !
import os


def main():
    read_all_files(".", True)


def read_all_files(folder, root_folder: bool, nesting_depth: int = 0):
    for file in os.listdir(folder):
        if file in [".git", ".idea", ".project", ".settings"]:
            continue
        if ".py" in file:
            continue
        if file not in ["framework"] and root_folder:
            continue
        if os.path.isdir(os.path.join(folder, file)):
            read_all_files(os.path.join(folder, file), False, nesting_depth + 1)
            continue
        print(os.path.join(folder, file))
        write = False
        with open(os.path.join(folder, file), "r") as in_file:
            rows = [x for x in in_file]
            for index, row in enumerate(rows):
                if "#include" in row:
                    if folder[2:] in row:
                        if "<config/" in row:
                            continue
                        write = True
                        row = row.replace(">", "\"")
                        row = row.replace("<framework", "\"")
                        row = row.replace(folder[1:] + "/", "")
                        rows[index] = row
                        print("Replacement 1" + str(row))
                    elif "<framework" in row:
                        how_deep = nesting_depth - 1
                        write = True
                        row = row.replace(">", "\"")
                        string = [".." for i in range(how_deep)]
                        dots = "/".join(string)
                        row = row.replace("<framework", "\"" + dots)
                        rows[index] = row
                        print("Replacement 2: " + str(row))
        if write:
            with open(os.path.join(folder, file), "w") as in_file:
                print("Write file %s" % (os.path.join(folder, file)))
                in_file.writelines(rows)


read_all_files(".", True)
