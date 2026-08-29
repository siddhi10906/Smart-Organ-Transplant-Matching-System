Build (Windows, MinGW-w64):

1) Install GCC (MinGW-w64) and ensure gcc is in PATH.
2) From project root:
   gcc -O2 -std=c11 -I c_engine\include c_engine\src\*.c -o c_engine\bin\transplant_engine.exe

Run:
- Start Node server: npm install, then npm start
- Use the web UI to add patients/donors, then click Run Matching.

Notes:
- The engine reads patients.txt and donors.txt from the project root.
- Hospital IDs are treated as 1-based in files and converted to 0-based internally.
