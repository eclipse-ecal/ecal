# Publisher Crash Test (pub\_crash)

This test verifies that communication remains intact even if one of the publishers crashes during execution.

* Two publishers are launched:

  * `crash_publisher` sends a few messages and then crashes.
  * `test_publisher` continues sending messages throughout the test.
* One subscriber (`subscriber`) listens on the shared topic.
* A monitoring process observes the presence of the processes.

**Success Criteria:**

* The subscriber continues to receive messages from `test_publisher` after `crash_publisher` terminates.
* Subscriber exits with success code if communication continuity is ensured.

---