# Peer-to-Peer Group Based File Sharing System

It is a group based file sharing system where users can share, download files from the group they belong to. Download should be parallel with multiple pieces from multiple peers.

## Commands to Run tracker and client

    Command to compile tracker : g++ tracker.cpp -o tracker -pthread
    
    Command to run tracker : ./tracker tracker_info.txt

    Command to compile client : g++ client.cpp -o client -pthread -lcrypto

    Command to run client : ./client <Client IP>:<Client PORT> tracker_info.txt 

Note:- tracker_info.txt contains the ip, port details of all tracker

## Commands to Run

a. Create User Account: create_user <user_id> <password>

b. Login: login <user_id> <password>

c. Create Group: create_group <group_id>

d. Join Group: join_group <group_id>

e. Leave Group: leave_group <group_id>

f. List pending join: list_requests <group_id>

g. Accept Group Joining Request: accept_request <group_id> <user_id>

h. List All Group In Network: list_groups

i. List All sharable Files In Group: list_files <group_id>

j. Upload File: upload_file <file_path> <group_id>

e.g.
for absolute path : upload_file /home/rishabh/IIIT/Yuktahar.pdf g1
for relative path : upload_file ../../reporting.pdf g1

k. Download File: download_file <group_id> <file_name> <destination_path>

e.g.
for absolute path : download_file g1 Yuktahar.pdf /home/rishabh/Desktop
for relative path : download_file g1 reporting.pdf ../../../


l. Logout: logout

m. Show_downloads: show_downloads

n. Stop Sharing : stop_share <group_id> <file_name> 



Note:-LogFile for the tracker

        logFile.txt will be created inside the tracker folder. It contains the tracker logs which are created during execution.
        To create a file,tracker should close with the "quit" command. If abruptly closed then logFile.txt will not be made.It makes the log in append mode.
     
     
       


