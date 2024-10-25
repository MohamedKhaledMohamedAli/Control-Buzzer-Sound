# To be able to use terminal from inside of Python file we import this library
import os

# To Make GUI and Display window
import tkinter

# To make Inter Process Communication (IPC)
import posix_ipc

# To be able to use the Shared Memory
import mmap

# Function SendValue that is called when push Button is pressed
def SendValue():
    
    # Get Value of Frequency
    # we use .strip() to remove unwanted spaces before the value being entered
    # To send the data to the shared memory it must be sent in a certain format ("utf-8") thats why we use .encode("utf-8")
    freq = entry_widget.get().strip().encode("utf-8")
    
    # To send the data to shared memory without offset (from beginning of the shared memory)
    # We need start sending data to the shared memory from byte 0 (first address)
    mm.seek(0)
    
    # To send data to Shared Memory
    # We add to the value we want to send "(b'\x00')" to act as a Null character
    mm.write(freq+b'\x00')
    
    # Good practice to make flush after writing in the shared memory
    mm.flush()

################### Shared Memory ###################
# We will give the Shared memory a name, and we try to make '/' the first character of the shared memory name
SHM_NAME = "/GUI"

# Size of Shared memory is in bytes --> we will make a shared memory of 4KByte = 4KB
SHM_SIZE = 4096

# To create object (file) of the Shared Memory
shm = posix_ipc.SharedMemory(SHM_NAME, posix_ipc.O_CREAT, size=SHM_SIZE)

# We will make map to the Shared memory to be able to communicate using it with C++ code
# .fd() to get the file descriptor of the shared memory
mm = mmap.mmap(shm.fd, SHM_SIZE)

try:
    
    ################### GUI ###################
    # Create Main Window
    main_window = tkinter.Tk()
    
    # Give the Main Window a Title
    main_window.title("Frequency Control For Buzzer")
    
    # Make dimensions of window
    main_window.geometry("400x100")
    
    # Don't allow the user to be able to Resize the window
    main_window.resizable(False, False)
    
    # Give the background a color
    main_window.configure(bg="green")
    
    # To make Label on the main window and make it's color white and change font 
    # and we will determine it's position in the main window
    label_widget = tkinter.Label(main_window, text="Enter the Frequency:", fg="white", bg="blue", font=("Arial", 14))
    label_widget.place(x=10, y=10)
    
    # To make line edit (in python called entry) to take input from the User
    entry_widget = tkinter.Entry(main_window, fg="white", bg="blue", font=("Arial", 14))
    entry_widget.place(x=180, y=10)
    
    # To make push button, command parameter is to call the function when the button is pressed
    pushButton_widget = tkinter.Button(main_window, text="Send the Frequency", fg="white", bg="blue", command=SendValue, font=("Arial", 20))
    pushButton_widget.place(x=80, y=50)
    
    # To make an infinite loop for the window
    main_window.mainloop()
    
# We go to finally when we press "x" at the window
finally:
    
    # Close Memory Mapping
    mm.close()
    
    # Close Shared memory file descriptor
    shm.close_fd()
    
    # Clear the terminal
    os.system("clear")
    
    # Print Goodbye in a different format
    print("""
    🅶🅾🅾🅳🅱🆈🅴
    """)