#!python
# -*- mode: python; Encoding: utf-8; coding: utf-8 -*-
# Last updated: <2024/04/16 02:36:29 +0900>
"""
wavefront obj to c language array

Windows10 x64 22H2 + Python 3.10.10 64bit
by mieki256
License: CC0 / Public Domain
"""

import argparse
import sys
import os


def dump_vertex(lbl, vtxs):
    print("// ----------------------------------------")
    print("// vertex")
    print("const unsigned int %s_vtx_size = %d;\n" % (lbl, len(vtxs)))
    print("const float %s_vtx[%d][3] = {" % (lbl, len(vtxs)))
    for v in vtxs:
        print("    { %s, %s, %s }, " % (v["x"], v["y"], v["z"]))
    print("};\n")


def dump_uv(lbl, uvs):
    print("// ----------------------------------------")
    print("// uv")
    print("const unsigned int %s_uv_size = %d;\n" % (lbl,  len(uvs)))
    print("const float %s_uv[%d][2] = {" % (lbl,  len(uvs)))
    for v in uvs:
        print("    { %s, %s }, " % (v["x"], v["y"]))
    print("};\n")


def dump_normal(lbl, nmls):
    print("// ----------------------------------------")
    print("// normal")
    print("const unsigned int %s_nml_size = %d;\n" % (lbl, len(nmls)))
    print("const float %s_nml[%d][3] = {" % (lbl, len(nmls)))
    for v in nmls:
        print("    { %s, %s, %s }, " % (v["x"], v["y"], v["z"]))
    print("};\n")


def dump_color(lbl, cols):
    print("// ----------------------------------------")
    print("// color")
    print("const unsigned int %s_col_size = %d;\n" % (lbl, len(cols)))
    print("const float %s_col[%d][4] = {" % (lbl, len(cols)))
    for v in cols:
        print("    { %s, %s, %s, %s }, " % (v[0], v[1], v[2], v[3]))
    print("};\n")


def dump_matrial_kd(lbl, mats, matdata):
    print("// material kd")
    print("const unsigned int %s_col_size = %d;" % (lbl, len(mats)))
    print("const float %s_col[%d][3] = {" % (lbl, len(mats)))
    for key in mats:
        kd = matdata[key]["Kd"]
        print("    { %s, %s, %s }, // %s" % (kd["r"], kd["g"], kd["b"], key))
    print("};\n")


def output_c_array(label, vtx_ary, uv_ary, nml_ary, col_ary, face_size):
    # output c language array
    labelupper = label.upper()
    print("\n#ifndef __%s_H_" % labelupper)
    print("#define __%s_H_\n" % labelupper)

    dump_vertex(label, vtx_ary)

    if len(uv_ary) > 0:
        dump_uv(label, uv_ary)

    if len(nml_ary) > 0:
        dump_normal(label, nml_ary)

    dump_color(label, col_ary)

    print("const unsigned int %s_face_size = %d;\n" % (label, face_size))

    print("#endif")


def main():
    infile = ""
    parser = argparse.ArgumentParser(description="wavefornt obj to c language array")
    parser.add_argument("infile", help="input wavefront obj file")
    # parser.add_argument("-i", "--input", help="input wavefront obj file")
    args = parser.parse_args()

    if args.infile is None:
        sys.exit()
    else:
        infile = args.infile

    dirname = os.path.dirname(infile)
    label = os.path.basename(infile).replace(".", "_")

    matfile = ""
    objname = ""
    groupname = ""

    vtxs = []
    uvs = []
    nmls = []
    faces = []

    use_nml = False
    use_uv = False
    use_smooth = False

    matname = ""
    mats = []

    # read obj file
    with open(infile, "r") as file:
        for line in file:
            line = line.rstrip()
            if line == "":
                continue

            if line[0] == "#":
                # found comment
                continue

            s = line.split()
            if s[0] == "mtllib":
                # material file
                matfile = s[1]
                print("// material_file=%s" % (matfile))
            elif s[0] == "o":
                # object name
                objname = s[1]
                print("// object_name=%s" % objname)
            elif s[0] == "g":
                # group name
                groupname = s[1]
                print("// group_name=%s" % groupname)
            elif s[0] == "v":
                # vertex
                vtxs.append({"x": s[1], "y": s[2], "z": s[3]})
            elif s[0] == "vt":
                # texture uv
                use_uv = True
                uvs.append({"x": s[1], "y": s[2]})
            elif s[0] == "vn":
                # normal
                use_nml = True
                nmls.append({"x": s[1], "y": s[2], "z": s[3]})
            elif s[0] == "usemtl":
                # use material
                matname = s[1]
                # print("// use_material=%s" % matname)
                if matname not in mats:
                    mats.append(matname)
            elif s[0] == "s":
                # smooth
                if True:
                    continue
                else:
                    use_smooth = False
                    if s[1] == "1":
                        use_smooth = True
            elif s[0] == "f":
                # face
                vtxi = []
                nmli = []
                uvi = []
                for idx in s:
                    if idx == "f":
                        continue
                    ix = idx.split("/")

                    if (not use_uv) and (not use_nml):
                        vi = int(ix[0]) - 1
                        vtxi.append(vi)
                    elif use_uv and (not use_nml):
                        vi = int(ix[0]) - 1
                        ti = int(ix[1]) - 1
                        vtxi.append(vi)
                        uvi.append(ti)
                    elif (not use_uv) and use_nml:
                        vi = int(ix[0]) - 1
                        ni = int(ix[2]) - 1
                        vtxi.append(vi)
                        nmli.append(ni)
                    else:
                        vi = int(ix[0]) - 1
                        ti = int(ix[1]) - 1
                        ni = int(ix[2]) - 1
                        vtxi.append(vi)
                        uvi.append(ti)
                        nmli.append(ni)

                if (not use_uv) and (not use_nml):
                    faces.append({"mat": matname, "v": vtxi})
                elif use_uv and (not use_nml):
                    faces.append({"mat": matname, "v": vtxi, "vt": uvi})
                elif (not use_uv) and use_nml:
                    faces.append({"mat": matname, "v": vtxi, "vn": nmli})
                else:
                    faces.append({"mat": matname, "v": vtxi, "vt": uvi, "vn": nmli})
            else:
                print(line)

    # read mtl file
    if matfile != "":
        matdata = {}
        matname = ""
        with open(os.path.join(dirname, matfile), "r") as file:
            for line in file:
                line = line.rstrip()
                if line == "":
                    continue

                if line[0] == "#":
                    # found comment
                    continue

                s = line.split()
                if s[0] == "newmtl":
                    matname = s[1]
                    matdata[matname] = {}
                elif s[0] == "Kd":
                    matdata[matname]["Kd"] = {"r": s[1], "g": s[2], "b": s[3]}
                elif s[0] == "Ka":
                    matdata[matname]["Ka"] = {"r": s[1], "g": s[2], "b": s[3]}
                elif s[0] == "Ks":
                    matdata[matname]["Ks"] = {"r": s[1], "g": s[2], "b": s[3]}
                elif s[0] == "Ns":
                    matdata[matname]["Ns"] = {"v": s[1]}
                elif s[0] == "d":
                    matdata[matname]["d"] = {"v": s[1]}
                elif s[0] == "Ni":
                    matdata[matname]["Ni"] = {"v": s[1]}
                elif s[0] == "illum":
                    matdata[matname]["illum"] = {"v": s[1]}

    # for v in uvs:
    #     print(v)

    # for v in faces:
    #     print(v)

    vtx_ary = []
    uv_ary = []
    nml_ary = []
    col_ary = []
    face_size = len(faces)

    for face in faces:
        matname = face["mat"]
        col = matdata[matname]["Kd"]
        r, g, b = col["r"], col["g"], col["b"]
        for i in face["v"]:
            vtx_ary.append(vtxs[i])
            col_ary.append([r, g, b, 1.0])

        if use_uv:
            for i in face["vt"]:
                uv_ary.append(uvs[i])

        if use_nml:
            for i in face["vn"]:
                nml_ary.append(nmls[i])

    output_c_array(label, vtx_ary, uv_ary, nml_ary, col_ary, face_size)


if __name__ == "__main__":
    main()
