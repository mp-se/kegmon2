Import("env")
import shutil

def get_build_flag_value(flag_name):
    build_flags = env.ParseFlags(env['BUILD_FLAGS'])
    flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
    defines = {k: v for (k, v) in flags_with_value_list}
    return defines.get(flag_name)

def after_build(source, target, env):
    print( "Executing custom step " )
    dir    = env.GetLaunchDir()
    name   = env.get( "PIOENV" )

    if name == "kegmon-esp32s3-pro-tft-sd" :
        target = dir + "/bin/firmware-32s3pro.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partition-32s3pro.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    if name == "kegmon-esp32s3-pro-tft-9341-sd" :
        target = dir + "/bin/firmware-32s3pro-9341.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partition-32s3pro.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    if name == "kegmon-esp32s3-pro-tft-9488-sd" :
        target = dir + "/bin/firmware-32s3pro-9488.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partition-32s3pro.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

print("Adding custom build step (copy firmware):")
# Target the generated binary directly. Keep this as a SCons Action rather
# than PlatformIO's VerboseAction: PIOVERBOSE unwraps VerboseAction back to a
# bare callback, which leaves the command text as Null in SCons 4.11.
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.Action(after_build, "Copying firmware artifacts"),
)
