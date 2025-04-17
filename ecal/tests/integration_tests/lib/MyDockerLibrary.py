import docker
from robot.api.deco import keyword
from robot.libraries.BuiltIn import BuiltIn
from docker.errors import NotFound

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
            remove=True,
            network=network,
        )
        self.containers[name] = container
        return container.id

    @keyword
    def stop_container(self, name):
        if name in self.containers:
            try:
                self.containers[name].stop()
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
            BuiltIn().log_to_console(f"\033[92m[✓] {test_name}: PASS\033[0m")
        else:
            BuiltIn().log_to_console(f"\033[91m[✗] {test_name}: FAIL\033[0m")
