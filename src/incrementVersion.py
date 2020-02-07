file = []
with open("src/version.h", "r") as fin:
    file += fin.readlines()
    for idx, line in enumerate(file):
        if line.find("#define VERSION_BUILD") != -1:
            l = line.split(" ")
            file[idx] = l[0] + " " +l[1]+ "               " + str(int(l[-1].strip('\n'))+1) + "\n"

with open("src/version.h", "w") as fout:
    for line in file:
        print(line)
        fout.write(line)
