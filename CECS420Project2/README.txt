Tyler Hamilton, SID 1713904

I believe the project is fully implemented. However, the program does not work when run with a buffer size of 1.

My messages are structured with an mtype, path, and keyword. The mtype is 1, and the path and keyword char arrays are limited by MAXDIRPATH and MAXKEYWORD respectively. Each line in the input file is read into a message. The message size was MAXDIRPATH + MAXKEYWORD, which should be equal to sizeof(message) - sizeof(long).
