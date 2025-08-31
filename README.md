# ConnectHub

A simple **console-based social network application** written in C.  
This project allows users to register, log in, send/accept/reject friend requests, manage friends, view statistics, and delete accounts.  
All user data, connections, and friend requests are persisted in text files.

---

## ✨ Features
- **User Registration & Login**
  - Register new users with username & password.
  - Login with saved credentials.
  - Tracks login count and last login timestamp.

- **Friend System**
  - Send, accept, or reject friend requests.
  - View current friends.
  - Remove friends.
  - Suggest friends (based on mutual connections).

- **User Management**
  - View user statistics (friends, login count, last login).
  - Delete account (soft delete, marks user inactive).
  
- **Persistence**
  - Users, connections, and requests are saved in text files (`users.txt`, `connections.txt`, `requests.txt`).

---


## ⚙️ Compilation & Execution

### Compile:
  ```bash
 gcc pbl.c -o social_network

 ./social_network





