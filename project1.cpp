
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <pwd.h>
#include <grp.h>

using namespace std;

class FileExplorer {
private:
    string currentPath;
    
    // Helper function to get file permissions string
    string getPermissionsString(mode_t mode) {
        string perms = "";
        perms += (S_ISDIR(mode)) ? "d" : "-";
        perms += (mode & S_IRUSR) ? "r" : "-";
        perms += (mode & S_IWUSR) ? "w" : "-";
        perms += (mode & S_IXUSR) ? "x" : "-";
        perms += (mode & S_IRGRP) ? "r" : "-";
        perms += (mode & S_IWGRP) ? "w" : "-";
        perms += (mode & S_IXGRP) ? "x" : "-";
        perms += (mode & S_IROTH) ? "r" : "-";
        perms += (mode & S_IWOTH) ? "w" : "-";
        perms += (mode & S_IXOTH) ? "x" : "-";
        return perms;
    }
    
    // Helper function to get file size in readable format
    string getReadableSize(off_t size) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double readableSize = size;
        
        while (readableSize >= 1024 && unitIndex < 4) {
            readableSize /= 1024;
            unitIndex++;
        }
        
        char buffer[50];
        sprintf(buffer, "%.2f %s", readableSize, units[unitIndex]);
        return string(buffer);
    }

public:
    FileExplorer() {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            currentPath = string(cwd);
        } else {
            currentPath = "/";
        }
    }
    
    // Day 1: List files in directory
    void listFiles(bool detailed = false) {
        DIR* dir = opendir(currentPath.c_str());
        if (dir == NULL) {
            cout << "Error opening directory: " << currentPath << endl;
            return;
        }
        
        cout << "\n========================================" << endl;
        cout << "Current Directory: " << currentPath << endl;
        cout << "========================================" << endl;
        
        struct dirent* entry;
        vector<string> files;
        vector<string> directories;
        
        while ((entry = readdir(dir)) != NULL) {
            string name = entry->d_name;
            
            // Skip . and ..
            if (name == "." || name == "..") continue;
            
            string fullPath = currentPath + "/" + name;
            struct stat fileStat;
            
            if (stat(fullPath.c_str(), &fileStat) == 0) {
                if (S_ISDIR(fileStat.st_mode)) {
                    directories.push_back(name);
                } else {
                    files.push_back(name);
                }
            }
        }
        
        closedir(dir);
        
        // Sort alphabetically
        sort(directories.begin(), directories.end());
        sort(files.begin(), files.end());
        
        // Display directories
        cout << "\nDirectories:" << endl;
        for (const auto& dir : directories) {
            if (detailed) {
                string fullPath = currentPath + "/" + dir;
                struct stat fileStat;
                stat(fullPath.c_str(), &fileStat);
                cout << "  [DIR]  " << setw(20) << left << dir 
                     << " | " << getPermissionsString(fileStat.st_mode) << endl;
            } else {
                cout << "  [DIR]  " << dir << endl;
            }
        }
        
        // Display files
        cout << "\nFiles:" << endl;
        for (const auto& file : files) {
            if (detailed) {
                string fullPath = currentPath + "/" + file;
                struct stat fileStat;
                stat(fullPath.c_str(), &fileStat);
                cout << "  [FILE] " << setw(20) << left << file 
                     << " | " << getPermissionsString(fileStat.st_mode)
                     << " | " << getReadableSize(fileStat.st_size) << endl;
            } else {
                cout << "  [FILE] " << file << endl;
            }
        }
        
        cout << "\nTotal: " << directories.size() << " directories, " 
             << files.size() << " files" << endl;
    }
    
    // Day 2: Navigation features
    void changeDirectory(const string& path) {
        string newPath;
        
        if (path == "..") {
            // Go to parent directory
            size_t lastSlash = currentPath.find_last_of('/');
            if (lastSlash != string::npos && lastSlash > 0) {
                newPath = currentPath.substr(0, lastSlash);
            } else {
                newPath = "/";
            }
        } else if (path[0] == '/') {
            // Absolute path
            newPath = path;
        } else {
            // Relative path
            newPath = currentPath + "/" + path;
        }
        
        struct stat fileStat;
        if (stat(newPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
            currentPath = newPath;
            cout << "Changed directory to: " << currentPath << endl;
        } else {
            cout << "Error: Directory not found or not accessible: " << path << endl;
        }
    }
    
    string getCurrentPath() const {
        return currentPath;
    }
    
    // Day 3: File manipulation
    bool copyFile(const string& source, const string& destination) {
        string srcPath = (source[0] == '/') ? source : currentPath + "/" + source;
        string destPath = (destination[0] == '/') ? destination : currentPath + "/" + destination;
        
        ifstream src(srcPath, ios::binary);
        if (!src) {
            cout << "Error: Cannot open source file: " << source << endl;
            return false;
        }
        
        ofstream dest(destPath, ios::binary);
        if (!dest) {
            cout << "Error: Cannot create destination file: " << destination << endl;
            return false;
        }
        
        dest << src.rdbuf();
        
        cout << "File copied successfully: " << source << " -> " << destination << endl;
        return true;
    }
    
    bool moveFile(const string& source, const string& destination) {
        string srcPath = (source[0] == '/') ? source : currentPath + "/" + source;
        string destPath = (destination[0] == '/') ? destination : currentPath + "/" + destination;
        
        if (rename(srcPath.c_str(), destPath.c_str()) == 0) {
            cout << "File moved successfully: " << source << " -> " << destination << endl;
            return true;
        } else {
            cout << "Error moving file: " << strerror(errno) << endl;
            return false;
        }
    }
    
    bool deleteFile(const string& filename) {
        string filePath = (filename[0] == '/') ? filename : currentPath + "/" + filename;
        
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) != 0) {
            cout << "Error: File not found: " << filename << endl;
            return false;
        }
        
        if (S_ISDIR(fileStat.st_mode)) {
            if (rmdir(filePath.c_str()) == 0) {
                cout << "Directory deleted successfully: " << filename << endl;
                return true;
            } else {
                cout << "Error deleting directory (may not be empty): " << strerror(errno) << endl;
                return false;
            }
        } else {
            if (remove(filePath.c_str()) == 0) {
                cout << "File deleted successfully: " << filename << endl;
                return true;
            } else {
                cout << "Error deleting file: " << strerror(errno) << endl;
                return false;
            }
        }
    }
    
    bool createFile(const string& filename) {
        string filePath = (filename[0] == '/') ? filename : currentPath + "/" + filename;
        
        ofstream file(filePath);
        if (file) {
            cout << "File created successfully: " << filename << endl;
            return true;
        } else {
            cout << "Error creating file: " << filename << endl;
            return false;
        }
    }
    
    bool createDirectory(const string& dirname) {
        string dirPath = (dirname[0] == '/') ? dirname : currentPath + "/" + dirname;
        
        if (mkdir(dirPath.c_str(), 0755) == 0) {
            cout << "Directory created successfully: " << dirname << endl;
            return true;
        } else {
            cout << "Error creating directory: " << strerror(errno) << endl;
            return false;
        }
    }
    
    // Day 4: Search functionality
    void searchFiles(const string& pattern, bool recursive = false) {
        cout << "\nSearching for: " << pattern << " in " << currentPath << endl;
        cout << "========================================" << endl;
        
        vector<string> results;
        searchInDirectory(currentPath, pattern, recursive, results);
        
        if (results.empty()) {
            cout << "No files found matching: " << pattern << endl;
        } else {
            cout << "Found " << results.size() << " match(es):" << endl;
            for (const auto& result : results) {
                cout << "  " << result << endl;
            }
        }
    }
    
private:
    void searchInDirectory(const string& path, const string& pattern, 
                          bool recursive, vector<string>& results) {
        DIR* dir = opendir(path.c_str());
        if (dir == NULL) return;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            string name = entry->d_name;
            
            if (name == "." || name == "..") continue;
            
            string fullPath = path + "/" + name;
            
            // Check if filename contains pattern
            if (name.find(pattern) != string::npos) {
                results.push_back(fullPath);
            }
            
            // Recursively search subdirectories
            if (recursive) {
                struct stat fileStat;
                if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
                    searchInDirectory(fullPath, pattern, recursive, results);
                }
            }
        }
        
        closedir(dir);
    }

public:
    // Day 5: Permission management
    void showPermissions(const string& filename) {
        string filePath = (filename[0] == '/') ? filename : currentPath + "/" + filename;
        
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) != 0) {
            cout << "Error: File not found: " << filename << endl;
            return;
        }
        
        cout << "\n========================================" << endl;
        cout << "File: " << filename << endl;
        cout << "========================================" << endl;
        cout << "Permissions: " << getPermissionsString(fileStat.st_mode) << endl;
        cout << "Octal: " << oct << (fileStat.st_mode & 0777) << dec << endl;
        
        // Get owner and group names
        struct passwd* pw = getpwuid(fileStat.st_uid);
        struct group* gr = getgrgid(fileStat.st_gid);
        
        cout << "Owner: " << (pw ? pw->pw_name : "unknown") << endl;
        cout << "Group: " << (gr ? gr->gr_name : "unknown") << endl;
        cout << "Size: " << getReadableSize(fileStat.st_size) << endl;
    }
    
    bool changePermissions(const string& filename, const string& permissions) {
        string filePath = (filename[0] == '/') ? filename : currentPath + "/" + filename;
        
        // Convert octal string to mode_t
        mode_t mode = 0;
        if (permissions.length() == 3) {
            for (int i = 0; i < 3; i++) {
                if (permissions[i] < '0' || permissions[i] > '7') {
                    cout << "Error: Invalid permissions format. Use octal (e.g., 755)" << endl;
                    return false;
                }
                mode = mode * 8 + (permissions[i] - '0');
            }
        } else {
            cout << "Error: Permissions must be 3 digits (e.g., 755)" << endl;
            return false;
        }
        
        if (chmod(filePath.c_str(), mode) == 0) {
            cout << "Permissions changed successfully for: " << filename << endl;
            return true;
        } else {
            cout << "Error changing permissions: " << strerror(errno) << endl;
            return false;
        }
    }
};

void displayMenu() {
    cout << "\n╔════════════════════════════════════════╗" << endl;
    cout << "║    LINUX FILE EXPLORER APPLICATION    ║" << endl;
    cout << "╚════════════════════════════════════════╝" << endl;
    cout << "\n[Navigation & Listing]" << endl;
    cout << "  1.  List files (simple)" << endl;
    cout << "  2.  List files (detailed)" << endl;
    cout << "  3.  Change directory" << endl;
    cout << "\n[File Operations]" << endl;
    cout << "  4.  Create file" << endl;
    cout << "  5.  Create directory" << endl;
    cout << "  6.  Copy file" << endl;
    cout << "  7.  Move file" << endl;
    cout << "  8.  Delete file/directory" << endl;
    cout << "\n[Search]" << endl;
    cout << "  9.  Search files (current directory)" << endl;
    cout << "  10. Search files (recursive)" << endl;
    cout << "\n[Permissions]" << endl;
    cout << "  11. Show file permissions" << endl;
    cout << "  12. Change file permissions" << endl;
    cout << "\n  0.  Exit" << endl;
    cout << "\n========================================" << endl;
}

int main() {
    FileExplorer explorer;
    int choice;
    string input, input2;
    
    cout << "Welcome to Linux File Explorer!" << endl;
    
    while (true) {
        displayMenu();
        cout << "Current Path: " << explorer.getCurrentPath() << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(); // Clear newline from buffer
        
        switch (choice) {
            case 1:
                explorer.listFiles(false);
                break;
                
            case 2:
                explorer.listFiles(true);
                break;
                
            case 3:
                cout << "Enter directory path (or .. for parent): ";
                getline(cin, input);
                explorer.changeDirectory(input);
                break;
                
            case 4:
                cout << "Enter filename to create: ";
                getline(cin, input);
                explorer.createFile(input);
                break;
                
            case 5:
                cout << "Enter directory name to create: ";
                getline(cin, input);
                explorer.createDirectory(input);
                break;
                
            case 6:
                cout << "Enter source file: ";
                getline(cin, input);
                cout << "Enter destination: ";
                getline(cin, input2);
                explorer.copyFile(input, input2);
                break;
                
            case 7:
                cout << "Enter source file: ";
                getline(cin, input);
                cout << "Enter destination: ";
                getline(cin, input2);
                explorer.moveFile(input, input2);
                break;
                
            case 8:
                cout << "Enter file/directory to delete: ";
                getline(cin, input);
                cout << "Are you sure? (y/n): ";
                getline(cin, input2);
                if (input2 == "y" || input2 == "Y") {
                    explorer.deleteFile(input);
                }
                break;
                
            case 9:
                cout << "Enter search pattern: ";
                getline(cin, input);
                explorer.searchFiles(input, false);
                break;
                
            case 10:
                cout << "Enter search pattern: ";
                getline(cin, input);
                explorer.searchFiles(input, true);
                break;
                
            case 11:
                cout << "Enter filename: ";
                getline(cin, input);
                explorer.showPermissions(input);
                break;
                
            case 12:
                cout << "Enter filename: ";
                getline(cin, input);
                cout << "Enter permissions (octal, e.g., 755): ";
                getline(cin, input2);
                explorer.changePermissions(input, input2);
                break;
                
            case 0:
                cout << "\nThank you for using Linux File Explorer!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice! Please try again." << endl;
        }
        
        cout << "\nPress Enter to continue...";
        cin.get();
    }
    
    return 0;
}

