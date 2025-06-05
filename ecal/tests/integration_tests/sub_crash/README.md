# Subscriber Crash Test (sub\_crash)

This test checks if communication continues when one of the subscribers crashes.

* One publisher (`test_publisher`) sends continuous messages.
* Two subscribers are launched:

  * `crash_subscriber` receives some messages and then crashes.
  * `test_subscriber` continues to receive messages throughout the test.
* A monitoring process tracks all running processes.

**Success Criteria:**

* `test_subscriber` continues receiving messages after `crash_subscriber` terminates.
* `test_subscriber` exits with a success code if it receives enough messages.
