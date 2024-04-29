# mqtt
## NAME
&emsp;mqtt - publishes or subscribes packets using MQTT protocol
## SYNOPSIS
&emsp;mqtt _--pub [-I user_id] [-N user_name] [-P password] [-b size] [-h host] [-m message] [-p port] [-t topic] [-v]_  
&emsp;mqtt _--sub [-I user_id] [-N user_name] [-P password] [-b size] [-h host] [-p port] [-t topic] [v]_  
## DESCRIPTION
&emsp;Connects to the broker using specified credentials and publishes to specified topic or starts waiting for subscribed topic.

&emsp;_-I user_id, --userid user_id_  
&emsp;&emsp;Uses specified user ID. By default randomly generated user id is used.  
&emsp;_-N user_name, --username user_name_  
&emsp;&emsp;Uses specified user name. By default none user name is iused.  
&emsp;_-P password, --password password_  
&emsp;&emsp;Uses specified password. By default none password is iused.  
&emsp;_-b size, --buffer-size size_  
&emsp;&emsp;Uses specified in bytes buffer size which is used to receive and send new packets. By default 1024 B size is iused.  
&emsp;_-h host, --host host_  
&emsp;&emsp;Uses specified host to connect to. By default localhost is used.  
&emsp;_-m message, --message message_  
&emsp;&emsp;Uses specified application message to publish it. By default empty message is used.  
&emsp;_-p port, --port port_  
&emsp;&emsp;Uses specified port number to connect to. By default 1884 port is used.  
&emsp;_-t topic, --topic topic_  
&emsp;&emsp;Uses specified topic to publish it or subscribe to it. This option is mandatory.  
&emsp;_-v, --verbose_  
&emsp;&emsp;Starts the program in a verbose mode.  
# Examples
1. Client establishes connection to the `test.mosquitto.org` without authentication and subscribes to '#'
```
mqtt --sub -h test.mosquitto.org -p 1883 -t '#' -v --mqtt-version 5
```
2. Client establishes connection to the `test.mosquitto.org` using login, password and subscribes to '#'
```
mqtt --sub -h test.mosquitto.org -p 1884 --username rw --password readwrite -t '#' -v --mqtt-version 4
```
3. Client establishes secure connection to the `test.mosquitto.org` without authentication, using only CA certificate and subscribes to '#'
```
mqtt --sub -h test.mosquitto.org -p 8883 -t '#' -v --mqtt-version 5 --cafile ./mosquitto.org.crt
```
4. Client establishes secure connection to the `test.mosquitto.org` without authentication, using CA certificate, his certificate, its private key and subscribes to '#'
```
mqtt --sub -h test.mosquitto.org -p 8884 -t '#' -v --mqtt-version 5 --cafile ./mosquitto.org.crt --cert ./client.crt --key ./client.key
```
5. Client establishes secure connection to the `test.mosquitto.org` using login, password, only CA certificate and subscribes to '#' 
```
mqtt --sub -h test.mosquitto.org -p 8885 --username rw --password readwrite -t '#' -v --mqtt-version 5 --cafile ./mosquitto.org.crt
```
