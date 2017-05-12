# -*- mode: python; -*-

# Return the pathname of the calling package.
# (This is used to recover the directory name to pass to cc -I<dir>, when
# choosing from among alternative header files for different platforms.)
def pkg_path_name():
    return "./" + Label(REPOSITORY_NAME + "//" + PACKAGE_NAME +
                        ":nsync").workspace_root + "/" + PACKAGE_NAME
