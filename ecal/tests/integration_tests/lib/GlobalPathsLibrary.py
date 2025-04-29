import os
from robot.api.deco import keyword

class GlobalPathsLibrary:
    def __init__(self):
        # Always resolve relative to the current .robot file location
        current_file_dir = os.path.dirname(os.path.abspath(__file__))
        # Go one level up from /lib to /integration_tests
        self.integration_tests_root = os.path.abspath(os.path.join(current_file_dir, ".."))

    @keyword
    def get_project_root(self):
        return self.integration_tests_root

    @keyword
    def get_configs_dir(self):
        return os.path.join(self.integration_tests_root, "cfg")

    @keyword
    def get_build_script_path(self):
        return os.path.join(self.integration_tests_root, "pub_sub_tests", "scripts", "build_pubsub_images.sh")

    @keyword
    def get_build_script_args(self):
        return ["basic_pub_sub", "basic_pub_sub"]

    @keyword
    def get_network_name(self):
        return "ecal_test_net"
