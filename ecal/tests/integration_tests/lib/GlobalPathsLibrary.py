import os
from robot.api.deco import keyword

class GlobalPathsLibrary:
    def __init__(self):
        # Resolve to /integration_tests directory
        current_file_dir = os.path.dirname(os.path.abspath(__file__))
        self.integration_tests_root = os.path.abspath(os.path.join(current_file_dir, ".."))

        # Default values (can be overridden)
        self.test_case_folder = "basic_pub_sub"
        self.tag_prefix = "basic_pub_sub"

    @keyword
    def set_test_context(self, test_case_folder: str, tag_prefix: str):
        """
        Sets the test folder (e.g. basic_pub_sub or message_validation) and tag prefix (e.g. basic_pub_sub).
        Should be called once in Suite Setup if you're using non-default test cases.
        """
        if test_case_folder in ["basic_pub_sub","sub_send_crash", "multi_pub_sub", "pub_crash", "sub_crash", "network_crash", "message_validation"]:
            self.test_case_folder = test_case_folder
            self.tag_prefix = tag_prefix
        else:
            raise ValueError(f"Unknown test case folder: {test_case_folder}")

    @keyword
    def get_project_root(self):
        return self.integration_tests_root

    @keyword
    def get_configs_dir(self):
        return os.path.join(self.integration_tests_root, "cfg")

    @keyword
    def get_build_script_path(self):
        return os.path.join(self.integration_tests_root, self.test_case_folder, "scripts", "build_images.sh")

    @keyword
    def get_build_script_args(self):
        return [self.tag_prefix]

    @keyword
    def get_tag_prefix(self):
        return self.tag_prefix

    @keyword
    def get_network_name(self):
        return "ecal_test_net"
