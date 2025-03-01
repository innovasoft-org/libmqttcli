# mqtt
## NAME
&emsp;mqtt - publishes or subscribes packets using MQTT protocol
## SYNOPSIS
&emsp;mqtt _--pub [-b size] [--cafile file] [--capath dir] [--cert file] [-h host] [--key file] [-m message] [--mqtt-version version] [-P password] [-p port] [--reuse-addr] [-t topic] [-I user_id] [-N user_name] [-v]_  
&emsp;mqtt _--sub [-b size] [--cafile file] [--capath dir] [--cert file] [-h host] [--key file] [--mqtt-version version] [-P password] [-p port] [--reuse-addr] [-t topic] [-I user_id] [-N user_name] [-v]_  
## DESCRIPTION
&emsp;Connects to the broker using specified credentials and publishes to specified topic or starts waiting for subscribed topic.

&emsp;_-b size, --buffer-size size_  
&emsp;&emsp;Uses specified in bytes buffer size which is used to receive and send new packets. By default 1024 B size is used.  
&emsp;_--cafile file_  
&emsp;&emsp;Sets a path to the file of CA certificates in PEM format. By default none _cafile_ is used.  
&emsp;_--capath dir_  
&emsp;&emsp;Sets a directory containing CA certificates in PEM format. By default none _capath_ is used.  
&emsp;_--cert client's certificate_  
&emsp;&emsp;Sets the path to the clients's certificate in PEM format. By default none _cert_ is used.  
&emsp;_-h host, --host host_  
&emsp;&emsp;Uses specified host to connect to. By default localhost is used.  
&emsp;_--key private key_  
&emsp;&emsp;Sets the path to the clients's certificate private key. By default none _key_ is used.  
&emsp;_-m message, --message message_  
&emsp;&emsp;Uses specified application message to publish it. By default empty message is used.  
&emsp;_--mqtt-version version_  
&emsp;&emsp;Sets the MQTT protocol version to be used. Values `4` and `5` are allowed. By default `5` is used.  
&emsp;_-P password, --password password_  
&emsp;&emsp;Uses specified password. By default none password is iused.   
&emsp;_-p port, --port port_  
&emsp;&emsp;Uses specified port number to connect to. By default 1884 port is used.  
&emsp;_--reuse-addr_  
&emsp;&emsp;Indicates that the rules used in validating addresses supplied in a `bind` call should allow reuse of local addresses. By default it is disabled.  
&emsp;_-t topic, --topic topic_  
&emsp;&emsp;Uses specified topic to publish it or subscribe to it. This option is mandatory.  
&emsp;_-I user_id, --userid user_id_  
&emsp;&emsp;Uses specified user ID. By default randomly generated user id is used.  
&emsp;_-N user_name, --username user_name_  
&emsp;&emsp;Uses specified user name. By default none user name is iused.  
&emsp;_-v, --verbose_  
&emsp;&emsp;Starts the program in a verbose mode. By default this option is disabled.  

# Examples
1. Client establishes connection to the `test.mosquitto.org` server without authentication and subscribes to '#' topic
```
mqtt --sub -h test.mosquitto.org -p 1883 -t '#' -v --mqtt-version 5
```
2. Client establishes connection to the `test.mosquitto.org` server using login, password and subscribes to '#' topic
```
mqtt --sub -h test.mosquitto.org -p 1884 --username rw --password readwrite -t '#' -v --mqtt-version 4
```
3. Client establishes secure connection to the `test.mosquitto.org` server without authentication, using only CA certificate and subscribes to '#' topic
```
mqtt --sub -h test.mosquitto.org -p 8883 -t '#' -v --mqtt-version 5 --cafile ./mosquitto.org.crt
```
4. Client establishes secure connection to the `test.mosquitto.org` server without authentication, using CA certificate, his certificate, his private key and subscribes to '#' topic
```
mqtt --sub -h test.mosquitto.org -p 8884 -t '#' -v --mqtt-version 5 --cafile ./mosquitto.org.crt --cert ./client.crt --key ./client.key
```
5. Client establishes secure connection to the `test.mosquitto.org` server using login, password, only CA certificate and subscribes to '#' topic
```
mqtt --sub -h test.mosquitto.org -p 8885 --username rw --password readwrite -t '#' -v --mqtt-version 5 --cafile ./mosquitto.org.crt
```

# Screenshots
Typical flow from the console was presented on <a href="#fig01">Fig. 1</a>.

<p align="center">
  <a name="fig01"> 
  <img src="../../doc/mqtt_console.png" /> </br>
  <b>Fig. 1. mqtt program - console view. </b>
  </a>
</p>
