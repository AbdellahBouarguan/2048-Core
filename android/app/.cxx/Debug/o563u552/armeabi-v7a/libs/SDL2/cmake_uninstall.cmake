if (NOT EXISTS "/home/boua/Desktop/My_C_and_C++_env/my_games_project/fstGame/2048-Core/android/app/.cxx/Debug/o563u552/armeabi-v7a/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"/home/boua/Desktop/My_C_and_C++_env/my_games_project/fstGame/2048-Core/android/app/.cxx/Debug/o563u552/armeabi-v7a/install_manifest.txt\"")
endif(NOT EXISTS "/home/boua/Desktop/My_C_and_C++_env/my_games_project/fstGame/2048-Core/android/app/.cxx/Debug/o563u552/armeabi-v7a/install_manifest.txt")

file(READ "/home/boua/Desktop/My_C_and_C++_env/my_games_project/fstGame/2048-Core/android/app/.cxx/Debug/o563u552/armeabi-v7a/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach (file ${files})
    message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    execute_process(
        COMMAND /home/boua/Desktop/My_C_and_C++_env/my_games_project/fstGame/2048-Core/libs/android-sdk/cmake/3.22.1/bin/cmake -E remove "$ENV{DESTDIR}${file}"
        OUTPUT_VARIABLE rm_out
        RESULT_VARIABLE rm_retval
    )
    if(NOT ${rm_retval} EQUAL 0)
        message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif (NOT ${rm_retval} EQUAL 0)
endforeach(file)

