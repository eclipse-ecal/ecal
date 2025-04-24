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
    def start_container(self, name, image, command=None, network=None):
        try:
            existing = self.client.containers.get(name)
            existing.remove(force=True)
        except docker.errors.NotFound:
            pass

        container = self.client.containers.run(
            image=image,
            command=command,
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
                print(f"Container {name} already removed.")
            finally:
                self.containers.pop(name, None)


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
    def create_docker_network(self, name):
        networks = self.client.networks.list(names=[name])
        if not networks:
            self.client.networks.create(name)
            print(f"Created Docker network: {name}")
        else:
            print(f"Docker network already exists: {name}")

    @keyword
    def log_test_summary(self, test_name, result):
        if result:
            BuiltIn().log_to_console(f"[✓] {test_name}: PASS")
        else:
            BuiltIn().log_to_console(f"[✗] {test_name}: FAIL")

    @keyword
    def overwrite_ecal_config(self, image, config_filename):
        """Overwrite ecal.yaml using docker cp and a temporary container."""
        container_name = f"temp_{image.replace(':', '_')}"

        try:
            # Start a stopped container from the image
            subprocess.run(["docker", "create", "--name", container_name, image], check=True)

            # Copy the desired config file into the container as ecal.yaml
            subprocess.run(["docker", "cp", config_filename, f"{container_name}:/usr/local/etc/ecal/ecal.yaml"], check=True)

            # Commit the container as the same image
            subprocess.run(["docker", "commit", container_name, image], check=True)

            print(f"Successfully overwrote ecal.yaml in image: {image}")
        except subprocess.CalledProcessError as e:
            print(f"Failed to overwrite config: {e}")
        finally:
            subprocess.run(["docker", "rm", "-f", container_name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    @keyword
    def build_image_if_missing(self, tag, dockerfile_path, context_dir):
        try:
            self.client.images.get(tag)
            print(f"Image {tag} already exists. Skipping build.")
        except ImageNotFound:
            print(f"Image {tag} not found. Building...")
            dockerfile_abs_path = os.path.abspath(dockerfile_path)
            context_abs_path = os.path.abspath(context_dir)
            print(f" → Dockerfile: {dockerfile_abs_path}")
            print(f" → Context:    {context_abs_path}")
            self.client.images.build(
                path=context_abs_path,
                dockerfile=dockerfile_abs_path,
                tag=tag,
                rm=True
            )
            print(f"Built image {tag}.")
