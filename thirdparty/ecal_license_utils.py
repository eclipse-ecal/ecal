import subprocess

from enum import Enum

class include_type(Enum):
    SUBMODULE               = 1
    COPY_IN_REPO            = 2
    COPY_IN_REPO_MODIFIED   = 3
    FETCHED_BY_CMAKE        = 4
    WINDOWS_BINARIES        = 5
    LINUX_BINARIES          = 6
    PYTHON_BINDINGS         = 7
    DOCUMENTATION           = 8
    TESTING                 = 9

def get_copyright_from_file(file_path, skip_lines=0):
    """
    Extracts the copyright information from a file.
    The function reads the first 10 lines of the file and looks for a line that starts with 'Copyright'.
    If found, it returns the line; otherwise, it returns None.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            # Skip the required number of lines
            for _ in range(skip_lines):
                if not f.readline():
                    return None
            copyright_lines = []
            for _ in range(10):
                line = f.readline()
                if not line:
                    break
                line = line.strip().lstrip("/*# ").rstrip("*/# ").strip()
                if line.lower().startswith("copyright"):
                    copyright_lines.append(line)
                    # Continue reading until an empty line (after stripping)
                    for _ in range(9):  # up to 10 lines total
                        next_line = f.readline()
                        if not next_line:
                            break
                        next_line = next_line.strip().lstrip("/*# ").rstrip("*/# ").strip()
                        if not next_line:
                            break
                        copyright_lines.append(" ")
                        copyright_lines.append(next_line)
                    return " ".join(copyright_lines)
        return None
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return None

def extract_license_from_cpp_source_file(source_file_path):
    """
    Extract the license text from a source file.
    This function assumes that the license is present in the first few lines of
    the file as a C / C++ style comment. It will extract all consecutive lines
    of comment until it finds a non-comment line or the end of the file.
    """
    license_text = []
    with open(source_file_path, 'r', encoding='utf-8') as file:
        first_comment_found    = False
        non_comment_line_found = False
        is_multiline_comment   = False

        for line in file:
            line = line.strip()

            # Check for the start of a comment
            if line.startswith("/*") or line.startswith("//"):
                first_comment_found = True
                is_multiline_comment = line.startswith("/*")

            # Append the line to the license text if we are in a comment
            if first_comment_found:
                if is_multiline_comment:
                    # Remove multiline comment markersa
                    if line.startswith("/** "):
                        line = line[4:].strip()
                    elif line.startswith("/**"):
                        line = line[3:].strip()
                    elif line.startswith("/* "):
                        line = line[3:].strip()
                    elif line.endswith("*/"):
                        line = line[:-2].strip()
                        is_multiline_comment = False
                    elif line.startswith("/*"):
                        line = line[2:].strip()
                    elif line.startswith("* "):
                        line = line[2:].strip()
                    elif line.startswith("*"):
                        line = line[1:].strip()

                else:
                    # Remove single line comment markers
                    if line.startswith("// "):
                        line = line[3:].strip()
                    elif line.startswith("//"):
                        line = line[2:].strip()
                    else:
                        line = ""
                        non_comment_line_found = True

                # Append line to the license text
                license_text.append(line)

                if non_comment_line_found:
                    break

    return "\n".join(license_text) if license_text else None

def get_git_version_from_submodule(submodule_path):
    # Get Git Tag (if it exists)
    describe_result = subprocess.run("git describe --exact-match --tags", cwd=submodule_path, capture_output=True)
    git_ref = describe_result.stdout.decode("utf-8").strip()

    # Get commit hash (if tag does not exist)
    if not git_ref:
        rev_parse_result = subprocess.run("git rev-parse --verify HEAD", cwd=submodule_path, capture_output=True)
        git_ref = rev_parse_result.stdout.decode("utf-8").strip()

    return git_ref

def get_repo_url_from_submodule(submodule_path):
    # Get the remote URL
    remote_result = subprocess.run("git config --get remote.origin.url", cwd=submodule_path, capture_output=True)
    repo_url = remote_result.stdout.decode("utf-8").strip()

    # Strip the ".git" suffix if it exists
    if repo_url.endswith(".git"):
        repo_url = repo_url[:-4]

    return repo_url