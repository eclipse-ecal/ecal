import docker
import os
import subprocess
from robot.api.deco import keyword
from robot.libraries.BuiltIn import BuiltIn
from docker.errors import NotFound, ImageNotFound

class MyDockerLibrary:
    def __init__(self):
        self.client = docker.from_env()
        self.containers = {}

    @keyword
    def start_container(self, name, image, *command_parts, network=None):
        try:
            existing = self.client.containers.get(name)
            existing.remove(force=True)
        except docker.errors.NotFound:
            pass

        container = self.client.containers.run(
            image=image,
            command=list(command_parts),
            name=name,
            detach=True,
            remove=False,
            network=network,
        )
        self.containers[name] = container
        return container.id


    @keyword
    def stop_container(self, name):
        if name in self.containers:
            try:
                self.containers[name].stop()
                self.containers[name].remove()
            except NotFound:
                BuiltIn().log_to_console(f"Container {name} already removed.")
            finally:
                self.containers.pop(name, None)

    @keyword
    def wait_for_container_exit(self, name):
        if name in self.containers:
            container = self.containers[name]
            result = container.wait()
            status_code = result.get("StatusCode", -1)
            return status_code
        else:
            raise Exception(f"Container {name} not found.")


    @keyword
    def run_and_get_exit_code(self, name, command):
        """Runs a command in container and returns the exit code."""
        if name in self.containers:
            try:
                exec_result = self.containers[name].exec_run(command)
                return exec_result.exit_code
            except NotFound:
                BuiltIn().fail(f"Container {name} not found.")
        else:
            BuiltIn().fail(f"Container {name} is not managed.")

    @keyword
    def get_container_logs(self, name):
        if name in self.containers:
            try:
                return self.containers[name].logs().decode("utf-8")
            except NotFound:
                return f"Logs not available: container {name} not found."
        return ""

    @keyword
    def run_command_in_container(self, name, command):
        if name in self.containers:
            try:
                exec_result = self.containers[name].exec_run(command)
                return exec_result.output.decode("utf-8")
            except NotFound:
                return f"Container {name} not found."
        return ""

    @keyword
    def wait_for_log_message(self, container_name, message, timeout=15):
        """Waits until a certain log message appears in the container logs."""
        import time
        start_time = time.time()
        container = self.containers.get(container_name)

        if not container:
            raise Exception(f"Container {container_name} not found.")

        while True:
            logs = container.logs().decode("utf-8")
            if message in logs:
                BuiltIn().log_to_console(f"[✓] Found message '{message}' in {container_name}")
                return True
            if time.time() - start_time > timeout:
                raise Exception(f"Timeout waiting for log message '{message}' in container {container_name}.")
            time.sleep(0.5)

    @keyword
    def create_docker_network(self, name):
        networks = self.client.networks.list(names=[name])
        if not networks:
            self.client.networks.create(name)
            BuiltIn().log_to_console(f"[✓]Created Docker network: {name}")
        else:
            BuiltIn().log_to_console(f"Docker network already exists: {name}")

    @keyword
    def log_test_summary(self, test_name, result):
        if result:
            BuiltIn().log_to_console(f"[✓] {test_name}: PASS")
        else:
            BuiltIn().log_to_console(f"[✗] {test_name}: FAIL")

    @keyword
    def overwrite_ecal_config(self, image, config_filename):
        """Overwrite ecal.yaml in the Docker image with full error handling."""
        container_name = f"temp_{image.replace(':', '_')}"
        try:
            # Create a stopped temporary container
            create_result = subprocess.run(
                ["docker", "create", "--name", container_name, image],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            if create_result.returncode != 0:
                raise RuntimeError(f"Failed to create container: {create_result.stderr.strip()}")

            # Copy the config file into the container
            copy_result = subprocess.run(
                ["docker", "cp", config_filename, f"{container_name}:/usr/local/etc/ecal/ecal.yaml"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            if copy_result.returncode != 0:
                raise RuntimeError(f"Failed to copy ecal.yaml: {copy_result.stderr.strip()}")

            # Commit the temporary container to update the image
            commit_result = subprocess.run(
                ["docker", "commit", container_name, image],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            if commit_result.returncode != 0:
                raise RuntimeError(f"Failed to commit container: {commit_result.stderr.strip()}")

            BuiltIn().log_to_console(f"[✓] Successfully overwrote ecal.yaml in image: {image}")

        except Exception as e:
            print(f"ERROR: {str(e)}")
            BuiltIn().fail(f"Failed to overwrite ECAL config: {str(e)}")

        finally:
            # Always try to clean up
            subprocess.run(["docker", "rm", "-f", container_name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


    @keyword
    def build_image_if_missing(self, tag, dockerfile_path, context_dir):
        try:
            self.client.images.get(tag)
            BuiltIn().log_to_console(f"Image {tag} already exists. Skipping build.")
        except ImageNotFound:
            BuiltIn().log_to_console(f"Image {tag} not found. Building...")
            dockerfile_abs_path = os.path.abspath(dockerfile_path)
            context_abs_path = os.path.abspath(context_dir)
            BuiltIn().log_to_console(f" → Dockerfile: {dockerfile_abs_path}")
            BuiltIn().log_to_console(f" → Context:    {context_abs_path}")
            self.client.images.build(
                path=context_abs_path,
                dockerfile=dockerfile_abs_path,
                tag=tag,
                rm=True
            )
            BuiltIn().log_to_console(f"Built image {tag}.")
