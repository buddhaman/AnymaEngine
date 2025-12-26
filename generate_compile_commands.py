import json
import os
import glob

project_dir = os.path.dirname(os.path.abspath(__file__)).replace("\\", "/")

common_args = [
    "-std=c++20",
    "/Od",
    "/Zi",
    "/MT",
    "/EHsc",
    "/W3",
    "-I", "Src",
    "-I", "external/sdl2/include",
    "-I", "external/opengl",
    "-I", "external/imgui/include",
    "-I", "external/imgui/backends",
    "-I", "external/implot/include",
]

entries = []

# Add main source file
entries.append({
    "directory": project_dir,
    "command": "cl.exe " + " ".join(common_args + ["/c", "Src/Anyma.cpp"]),
    "file": "Src/Anyma.cpp"
})

# Add test runner
entries.append({
    "directory": project_dir,
    "command": "cl.exe " + " ".join(common_args + ["/c", "Test/TestRunner.cpp"]),
    "file": "Test/TestRunner.cpp"
})

# Automatically find all test files
test_files = glob.glob("Test/Src/*.cpp")
for test_file in test_files:
    test_file = test_file.replace("\\", "/")
    entries.append({
        "directory": project_dir,
        "command": "cl.exe " + " ".join(common_args + ["/c", test_file]),
        "file": test_file
    })

with open("compile_commands.json", "w") as f:
    json.dump(entries, f, indent=2)

print(f"Generated compile_commands.json with {len(entries)} entries")
