import sys
import re
import subprocess
import logging
from pathlib import Path
import shutil
import argparse

def setup_logging():
    """Sets up the logging configuration."""
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


def list_files(directory: Path, extension: str) -> list[Path]:
    """Returns a list of files with specific extension in the given directory."""
    if not directory.exists():
        logging.error(f"Directory {directory} does not exist.")
        return []
    return list(directory.glob(f"*{extension}"))


def copy_files(source_dir: Path, target_dir: Path, file_types: list[str]):
    """Copies files of specific types from source to target directory."""
    target_dir.mkdir(parents=True, exist_ok=True)
    for file_type in file_types:
        for file in source_dir.glob(file_type):
            try:
                shutil.copy2(file, target_dir)
                logging.info(f"Copied {file.name} to {target_dir}")
            except Exception as e:
                logging.error(f"Error copying {file.name} to {target_dir}: {e}")


def check_nanopb_compiler_exists(compiler_path: Path):
    """Checks if the nanopb_generator exists."""
    if not compiler_path.exists():
        logging.error(f"nanopb generator not found at {compiler_path}")
        sys.exit(1)


def run_nanopb_generator(proto_files: list[Path], compiler: Path, ecal_pb_base_path: Path, ecal_pb_sub_path: Path, target_dir: Path):
    """Runs the nanopb generator for each proto file."""
    for proto_file_path in proto_files:
        proto_file_name = proto_file_path.name
        
        command = [
            str(compiler),
            "-I" + str(ecal_pb_base_path),
            "-I" + str(ecal_pb_base_path / ecal_pb_sub_path),
            "-D" + str(target_dir / ecal_pb_sub_path),
            "-e" + ".npb",
            proto_file_name
        ]

        logging.info(f"Running: {' '.join(command)}")
        
        try:
            result = subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, cwd=proto_file_path.parent)
            logging.info(f"Success: {proto_file_name} processed.")
            logging.debug(result.stdout)
        except subprocess.CalledProcessError as e:
            logging.error(f"Error running nanopb_generator for {proto_file_name}: {e.stderr}")


def main(nano_pb_path: Path, ecal_repository: Path):
    setup_logging()

    # Define paths based on the provided arguments
    ecal_pb_base_path = Path(ecal_repository / "ecal/core_pb/src")
    ecal_pb_sub_path = Path("ecal/core/pb")
    ecal_target_path = Path("../../core/src/serialization/nanopb")

    # Combine paths and list .proto files
    proto_files_dir = ecal_pb_base_path / ecal_pb_sub_path
    ecal_pb_files = list_files(proto_files_dir, ".proto")
    
    # Check if nanopb generator exists
    nano_pb_compiler = nano_pb_path / "generator-bin/nanopb_generator.exe"
    check_nanopb_compiler_exists(nano_pb_compiler)

    # Check if any .proto files are found
    if not ecal_pb_files:
        logging.error(f"No .proto files found in {proto_files_dir}")
        sys.exit(1)

    # Prepare target directory
    absolute_target_path = (ecal_pb_base_path / ecal_target_path).resolve()
    absolute_target_path.mkdir(parents=True, exist_ok=True)

    # Copy nanopb common decoder and encoder to target nanopb directory
    target_nanopb_dir = absolute_target_path
    copy_files(nano_pb_path, target_nanopb_dir, ["*.c", "*.h"])

    # Run nanopb_generator for each .proto file
    run_nanopb_generator(ecal_pb_files, nano_pb_compiler, ecal_pb_base_path, ecal_pb_sub_path, absolute_target_path)


if __name__ == "__main__":
    # Set up argument parsing
    parser = argparse.ArgumentParser(description="Process nanopb and eCAL protobuf files.")
    parser.add_argument("--nano_pb_path", type=Path, required=True, help="Path to the nanopb directory")
    parser.add_argument("--ecal_repository", type=Path, required=True, help="Path to the eCAL repository")

    # Parse the arguments
    args = parser.parse_args()

    # Resolve paths to absolute paths to support relative paths
    nano_pb_path = Path(args.nano_pb_path).resolve()
    ecal_repository = Path(args.ecal_repository).resolve()

    # Call the main function with resolved absolute paths
    main(nano_pb_path, ecal_repository)