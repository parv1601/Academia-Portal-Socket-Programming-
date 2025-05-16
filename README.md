# Course Registration Portal (Academia)

## 1. Problem Statement

The goal of this project is to design and develop a **Course Registration Portal (Academia)** that is user-friendly and multifunctional. The portal should allow **Students, Faculty, and Administrators** to manage academic operations such as:

- Course registration
- Account management
- Course offerings

The system should ensure:

- **Data consistency and security** using file-based storage
- **Password-protected access**
- **File locking mechanisms**
- **Concurrent access** by multiple clients using **socket programming** and **multithreading**

---

## 2. Implementation Details (Approach)

### 2.1 Architecture

The system follows a **client-server architecture**:

- **Server**: Maintains the database (stored in files) and handles requests from multiple clients concurrently.
- **Clients**: Connect to the server and interact with the system based on their roles (**Admin, Faculty, or Student**).

---

### 2.2 Key Features

### Admin Features:

- Add, view, update, activate, or deactivate student and faculty accounts.
- Manage user accounts and ensure secure access.

### Faculty Features:

- Add or remove courses.
- View enrollments in courses.
- Manage course offerings.

### Student Features:

- Enroll in or drop courses.
- View enrolled courses.
- Change account password.

---

### 2.3 Concurrency

- Multiple clients can connect to the server **simultaneously**
- **File locking** ensures data consistency during concurrent access.

---

### 2.4 Security

- **Password-protected login** for all roles.
- **Role-based access control**.

---

### 2.5 File-Based Storage

| File Name | Description |
| --- | --- |
| `students.dat` | Stores student details |
| `faculty.dat` | Stores faculty details |
| `courses.dat` | Stores course details |
| `student_courses.dat` | Stores student-course relationships |

---

### 2.6 Synchronization

- **Mutex Locks**: Ensure thread-safe access to shared files.

---

### 2.7 Socket Programming

- The **server** listens for incoming client connections.
- Each client is handled in a **separate thread** to support concurrent access.

---

## 3. Source Code Snippets with Explanation

### 3.1 Server Initialization

```c
int start_server() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\\n", PORT);

    // Accept client connections
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create a thread to handle the client
        pthread_t thread_id;
        int *client_sock = malloc(sizeof(int));
        *client_sock = client_socket;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_sock);
        pthread_detach(thread_id);
    }

    return 0;
}

```

**Description:**

The server initializes a socket, binds it to a port, and listens for incoming connections. Each client is handled in a separate thread.

**Explanation:**

- The server creates a socket and binds it to a port.
- It listens for incoming connections and spawns a new thread for each client.

---

### 3.2 Admin: Add Student

```c
int add_student(Student *student) {
    pthread_mutex_lock(&student_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    int result = write(fd, student, sizeof(Student));

    close(fd);
    pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
    return result;
}

```

**Description:**

The admin can add a new student to the system. The student details are stored in `students.dat`.

**Explanation:**

- The function locks the `students.dat` file using a mutex to ensure thread safety.
- The student details are appended to the file.

---

### 3.3 Faculty: View Offered Courses

```c
void view_faculty_courses_helper(int client_socket, const char *faculty_id) {
    pthread_mutex_lock(&course_file_mutex); // Lock the course file mutex

    int fd = open(COURSE_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        send_message(client_socket, "Error accessing course records.\\n");
        return;
    }

    Course course;
    char buffer[BUFFER_SIZE];
    int count = 0;

    send_message(client_socket, "\\n=== Your Courses ===\\n");
    send_message(client_socket, "Code\\tName\\tCredits\\tAvailable Seats\\n");

    while (read(fd, &course, sizeof(Course)) > 0) {
        if (strcmp(course.faculty_id, faculty_id) == 0) {
            snprintf(buffer, BUFFER_SIZE, "%s\\t%s\\t%d\\t%d\\n",
                     course.course_code,
                     course.name,
                     course.credits,
                     course.available_seats);
            send_message(client_socket, buffer); // Send each line immediately
            count++;
        }
    }

    if (count == 0) {
        send_message(client_socket, "No courses found.\\n");
    }

    close(fd);
    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
}

```

**Description:**

The faculty can view the courses they are offering.

**Explanation:**

- The function locks the `courses.dat` file using a mutex.
- It reads the courses offered by the faculty and sends the details to the client.

---

### 3.4 Student: Enroll in Course

```c
int enroll_student_course(StudentCourse *sc) {
    pthread_mutex_lock(&student_course_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_COURSE_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        return -1; // File access error
    }

    StudentCourse temp_sc;
    while (read(fd, &temp_sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(temp_sc.student_id, sc->student_id) == 0 &&
            strcmp(temp_sc.course_code, sc->course_code) == 0 &&
            temp_sc.is_enrolled) {
            close(fd);
            pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
            return -2; // Already enrolled
        }
    }

    sc->is_enrolled = 1;
    if (write(fd, sc, sizeof(StudentCourse)) == -1) {
        close(fd);
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
        return -3; // Write error
    }

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
    return 1; // Success
}

```

**Description:**

The student can enroll in a course if seats are available.

**Explanation:**

- The function checks if the student is already enrolled in the course.
- If not, it adds the enrollment record to `student_courses.dat`.

---

### 3.5 Client-Side Interaction

```c
int enroll_student_course(StudentCourse *sc) {
    pthread_mutex_lock(&student_course_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_COURSE_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        return -1; // File access error
    }

    StudentCourse temp_sc;
    while (read(fd, &temp_sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(temp_sc.student_id, sc->student_id) == 0 &&
            strcmp(temp_sc.course_code, sc->course_code) == 0 &&
            temp_sc.is_enrolled) {
            close(fd);
            pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
            return -2; // Already enrolled
        }
    }

    sc->is_enrolled = 1;
    if (write(fd, sc, sizeof(StudentCourse)) == -1) {
        close(fd);
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
        return -3; // Write error
    }

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
    return 1; // Success
}

```

**Description:**

The client sends requests to the server and displays the server's responses.

**Explanation:**

- The client sends the user's choice to the server.
- It receives and displays the server's response.

---

### 4. MAKEFILE :

```bash
make #to compile all the source codes
make run_server #to run the server side
make run_client #to run the client side
make clean #to delete all the executables
```

---
