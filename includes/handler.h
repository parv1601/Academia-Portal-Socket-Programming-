#ifndef HANDLER_H
#define HANDLER_H

void admin_handler(int client_socket);

void faculty_handler(int client_socket,const char *faculty_id);

void student_handler(int client_socker,const char *student_id);


#endif