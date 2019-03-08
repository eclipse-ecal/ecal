/** 
 * @mainpage eCAL - enhanced communication abstraction layer
 * 
 * <h2>Introduction</h2>
 * 
 * The enhanced communication abstraction layer (eCAL) is a middleware that enables scalable, high performance interprocess 
 * communication on a single computer node or between different nodes in a computer network. The design is inspired by known
 * Data Distribution Service for Real-Time Systems (see Data distribution service on wikipedia). The current eCAL implementation
 * realizes a subset of such a DDS system, there is no support for Quality of Service (QoS) driven data transport.
 * <br>
 * eCAL is designed for typical cloud computing scenarios where different processes exchange there I/O's using a publisher/subscriber
 * pattern. The data exchange is based on so called topics. A topic wraps the payload that should be exchanged with additional informations
 * like a unique name, a type and a description. A topic can be connected to more then one publisher and/or subscriber. These are the basic
 * elements of the eCAL API.
 * <br>
 * <ul>
 * <li>Topic: The most basic description of the data to be published and subscribed.</li>
 * </ul>
 * <ul>
 * <li>Publisher: A Publisher is the object responsible for the actual dissemination of publications.</li>
 * </ul>
 * <ul>
 * <li>Subscriber: A Subscriber is the object responsible for the actual reception of the data resulting from its subscriptions.</li>
 * </ul>
 * <ul>
 * <li>Callback: A Callback can be used to react on incoming messages.</li>
 * </ul>
 * eCAL is simplifying the data transport as much as possible, It uses different mechanism to transport a topic from a publisher to a
 * connected subscriber. On the same computer node the data are exchanged by using memory mapped files. Between different computer nodes
 * UDP multicast is used to spread the data as fast as possible.
 * 
 * <h2>A simple hello world example</h2>
 * 
 * Now lets see a minimal publisher/subscriber example exchanging the famous "hello world" message :-). First the publisher ..
 * 
 * \code
 * #include <ecal/ecal.h>
 * 
 * int main(int argc, char **argv)
 * {
 *   // initialize eCAL API
 *   eCAL::Initialize(argc, argv, "minimal_pub");
 *
 *   // create a publisher (topic name "foo", type "std::string")
 *   eCAL::CPublisher pub("foo", "std::string");
 *
 *   // send the content
 *   pub.Send("hello world");
 *
 *   // finalize eCAL API
 *   eCAL::Finalize();
 * }
 * \endcode
 * 
 * and the minimal subscriber ..
 * 
 * \code
 * #include <ecal/ecal.h>
 * 
 * int main(int argc, char **argv)
 * {
 *   // initialize eCAL API
 *   eCAL::Initialize(argc, argv, "minimal_sub");
 *
 *   // create a subscriber (topic name "foo", type "std::string")
 *   eCAL::CSubscriber sub("foo", "std::string");
 *
 *   // receive content
 *   std::string msg;
 *   sub.Receive(msg);
 *
 *   // finalize eCAL API
 *   eCAL::Finalize();
 * }
 * \endcode
 * 
 * <h2>Trigger on incoming topics</h2>
 * 
 * A callback function can be used to react on events of multiple subscribers. An extended
 * "hello world" example could look like this. First the publisher part
 * 
 * \code
 *  // create 2 publishers
 *  eCAL::CPublisher pub1("foo1", "std::string");
 *  eCAL::CPublisher pub2("foo2", "std::string");
 *  
 *  // sending "hello world" on 2 different topics
 *  while(eCAL::Ok())
 *  {
 *    pub1.Send("hello");
 *    eCAL::Process::SleepMS(1000);
 *    pub2.Send("world");
 *  }
 * \endcode
 * 
 * We use a callback function to react on incoming topics (with "old school" function pointer)
 * 
 * \code
 *  // define a subscriber callback function
 *  void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_, const void* par_)
 *  {
 *    printf("We received %s on topic %s\n.", (char*)data_->buf, topic_name_.c_str());
 *  }
 *
 *  // create 2 subscriber
 *  eCAL::CSubscriber sub1("foo1", "std::string");
 *  eCAL::CSubscriber sub2("foo2", "std::string");
 *
 *  // register subscriber callback function
 *  sub1.AddReceiveCallback(OnReceive);
 *  sub2.AddReceiveCallback(OnReceive);
 *
 *  // idle main thread
 *  while(eCAL::Ok())
 *  {
 *    // sleep 100 ms
 *    std::this_thread::sleep_for(std::chrono::milliseconds(100));
 *  }
 * \endcode
 * 
 * or we can connect to a callback function using std::bind and a specialized string message subscriber
 * 
 * \code
 *  // define a subscriber callback function
 *  void OnReceive(const char* topic_name_, const std::string& message_)
 *  {
 *    printf("We received %s on topic %s\n.", message_.c_str(), topic_name_.c_str());
 *  }
 *
 *  // create 2 subscriber
 *  eCAL::string::CSubscriber sub1("foo1");
 *  eCAL::string::CSubscriber sub2("foo2");
 *
 *  // register subscriber callback function
 *  auto callback = std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2);
 *  sub1.AddReceiveCallback(callback);
 *  sub2.AddReceiveCallback(callback);
 *
 *  // idle main thread
 *  while(eCAL::Ok())
 *  {
 *    // sleep 100 ms
 *    std::this_thread::sleep_for(std::chrono::milliseconds(100));
 *  }
 * \endcode
 *
 * <h2>(De)Serialization of objects</h2>
 * 
 * The eCAL middleware does not provide a mechanism to serialize or deserialize message objects. This has to be implemented in a higher level API.
 * See <a href="https://google.github.io/flatbuffers/">google flatbuffers</a>, <a href="https://capnproto.org/">cap'n proto</a>
 * or <a href="http://code.google.com/p/protobuf/">google protobuf</a> for more details.
 * Here a typical example using <a href="http://code.google.com/p/protobuf/">google protobuf</a>. First you have to define a so called .proto file for
 * the structure you want to serialize and deserialize. A simple Shape struct will be defined as follows ..
 * 
 * \code
 *  message Shape
 *  {
 *    enum ShapeType
 *    {
 *      CIRCLE    = 0;
 *      TRIANGLE  = 1;
 *      RECTANGLE = 2;
 *    }
 *    required ShapeType type = 1 [default = CIRCLE];
 *    required int32     size = 2;
 *  }
 * \endcode
 *
 * After compiling that .proto file using the protobuf compiler you will receive a shape.pb.cc and shape.pb.h file that you have to include into your project. The serialization and
 * deserialization is then quite simple ..
 *
 * \code
 *  // create a publisher (topic name "shape")
 *  eCAL::protobuf::CPublisher<Shape> pub("shape");
 *
 *  // and generate a class instance of Shape
 *  class Shape shape;
 *
 *  // modify type and size of the shape object
 *  shape.set_type(Shape_ShapeType_CIRCLE);
 *  shape.set_size(42);
 *
 *  // send the shape object
 *  pub.Send(shape);
 * \endcode
 * 
 * And last not least the matching deserialization ..
 * 
 * \code
 *  // create a subscriber (topic name "shape")
 *  eCAL::protobuf::CSubscriber<Shape> sub("shape");
 *
 *  // and generate a class instance of Shape
 *  class Shape shape;
 *
 *  // receive the shape object
 *  sub.Receive(shape);
 * \endcode
 * 
 * A similar way can be used to work with google flatbuffers. The flatbuffer message schema looks like this
 * 
 * \code
 *  namespace Game.Sample;
 *  
 *  enum Color:byte { Red = 0, Green, Blue = 2 }
 *  
 *  union Any { Monster }  // add more elements..
 *  
 *  struct Vec3
 *  {
 *    x:float;
 *    y:float;
 *    z:float;
 *  }
 *  
 *  table Monster
 *  {
 *    pos:Vec3;
 *    mana:short = 150;
 *    hp:short = 100;
 *    name:string;
 *    friendly:bool = false (deprecated);
 *    inventory:[ubyte];
 *    color:Color = Blue;
 *  }
 *  
 *  root_type Monster;
 * \endcode
 *
 * After compiling the flatbuffer message we can send it the same way like a protocol buffer.
 *
 * \code
 *  // create a publisher (topic name "monster")
 *  eCAL::flatbuffers::CPublisher<flatbuffers::FlatBufferBuilder> pub("monster");
 *
 *  // the generic builder instance
 *  flatbuffers::FlatBufferBuilder builder;
 *
 *  // generate a class instance of Monster
 *  auto vec = Game::Sample::Vec3(1, 2, 3);
 *
 *  auto name = builder.CreateString("Monster");
 *
 *  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *  auto inventory = builder.CreateVector(inv_data, 10);
 *
 *  // shortcut for creating monster with all fields set:
 *  auto mloc = Game::Sample::CreateMonster(builder, &vec, 150, 80, name, inventory, Game::Sample::Color_Blue);
 *
 *  // finalize the message
 *  builder.Finish(mloc);
 *
 *  // and send the monster
 *  pub.Send(builder, -1);
 * \endcode
 * 
 * Finally let's receive it via receive callback. So first we define a callback function for the flatbuffer message.
 *
 * \code
 *  void OnMonster(const char* topic_name_, const flatbuffers::FlatBufferBuilder& msg_, const long long time_)
 *  {
 *    // create monster
 *    auto monster = Game::Sample::GetMonster(msg_.GetBufferPointer());
 *  
 *    // print content
 *    std::cout << "monster pos x     : " << monster->pos()->x()       << std::endl;
 *    std::cout << "monster pos y     : " << monster->pos()->y()       << std::endl;
 *    std::cout << "monster pos z     : " << monster->pos()->z()       << std::endl;
 *    std::cout << "monster name      : " << monster->name()->c_str()  << std::endl;
 *  }
 * \endcode
 * 
 * and implement the flatbuffer subscriber.
 *
 * \code
 *  // create a subscriber (topic name "monster")
 *  eCAL::flatbuffers::CSubscriber<flatbuffers::FlatBufferBuilder> sub("monster");
 *
 *  // add receive callback function (_1 = topic_name, _2 = msg, _3 = time)
 *  auto callback = std::bind(OnMonster, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
 *  sub.AddReceiveCallback(callback);
 * \endcode
 * 
 * <h2>Catching eCAL events</h2>
 * 
 * Beside the message receive callbacks eCAL provides an API to get informed if publisher / subscriber internal events get fired. The following
 * example shows how to catch the connect and disconnect events of an eCAL publisher. First we define the event callback function ..
 * 
 * \code
 *  void OnEvent(const char* topic_name_, const struct eCAL::SEventCallbackData* data_)
 *  {
 *    std::cout << "topic name   : " << topic_name_ << std::endl;
 *    switch (data_->type)
 *    {
 *    case pub_event_connected:
 *      std::cout << "event        : " << "pub_event_connected" << std::endl;
 *      break;
 *    case pub_event_disconnected:
 *      std::cout << "event        : " << "pub_event_disconnected" << std::endl;
 *      break;
 *    default:
 *      std::cout << "event        : " << "unknown" << std::endl;
 *      break;
 *    }
 *    std::cout << std::endl;
 *  }
 * \endcode
 * 
 * Then we only have to bind the event we are interested in to the event callback.
 * 
 * \code
 *  // create a publisher (topic name "person")
 *  eCAL::protobuf::CPublisher<class pb::People::Person> pub("person");
 *
 *  // add event callback function (_1 = topic_name, _2 = event data struct)
 *  auto evt_callback = std::bind(OnEvent, std::placeholders::_1, std::placeholders::_2);
 *  pub.AddEventCallback(pub_event_connected,    evt_callback);
 *  pub.AddEventCallback(pub_event_disconnected, evt_callback);
 * \endcode
**/
