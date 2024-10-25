#include <iostream>
#include <wiringPi.h> // Used for Pins Numbering for GPIO
#include <thread> // if we have multiple threads or to make delay
#include <atomic> // To avoid a shared variable from being accessed by more than one thread at a time
#include <unistd.h>
#include <cstring> // Used for char string functions

/************ Used for the Shared Memory ************/
// They carry Macros (#defines) & functions to use shared memory
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

// Shared Memory Name
const char *SHM_NAME = "/GUI";

// Shared Memory Size
const int SHM_SIZE = 4096;

// Buffer to receive data from shared memory
char* buffer = nullptr;

// Buffer to store old value to display values on change only
char old_buffer[SHM_SIZE] = "";

// File descriptor for the shared memory
int shm_fd;

// To avoid a shared variable from being accessed by more than one thread at a time
// This variable is used to close the program
std::atomic<bool> QuitFlag(true);

// Class to be responsible for the Buzzer
class Buzzer
{
    private:
        
        int buzzer_pin;
        
        // To play with PWM to make buzzer change it's tone
        int freq;
        
        // clock of peripheral that generate the PWM and has a default value = 19.2 MHz = 19,200,000
        int base_clock;
        
        // To play with PWM to make buzzer change it's tone
        double duty_cycle;
        
        // Can make 2 different cycles or more to be repeated, has default value = 1024
        int Clock_Range;
        
        // Output clock (signal) that will be sent to buzzer to control it's tone
        int PWM_clock;
    public:
        
        // Constructor
        Buzzer(int buzzer_pin): buzzer_pin(buzzer_pin), base_clock(19200000), duty_cycle(0.5), Clock_Range(1024)
        {
            
            // Set Buzzer pin as PWM output
            pinMode(buzzer_pin, PWM_OUTPUT);
            
            // Set the mode of PWM, we will chose PWM mode mark space (MS) to be able to change PWM signal as I want all the time
            pwmSetMode(PWM_MODE_MS);
            
            // Set PWM clock range
            pwmSetRange(Clock_Range);
        }
        
        // Function to change clock of the PWM --> As if we are making prescale to the PWM signal
        void set_frequency()
        {
            
            // Infinite Loop till program termination
            while(QuitFlag.load())
            {
                
                // We will use sscanf() to convert string to integer
                sscanf(buffer, "%d", &freq);
                
                // To Calculate prescale of the clock of the PWM
                PWM_clock = base_clock / (freq * Clock_Range);
                
                // To send the prescale of the clock of the PWM
                pwmSetClock(PWM_clock);
                
                // Make some delay as good practice
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // Function to change duty cycle of the PWM to change tone of buzzer
        // We change tone of buzzer by changing the prescale of the base clock of the PWM
        void run()
        {
            
            // Duty Cycle
            int Duty = duty_cycle * Clock_Range;
            
            // Infinite Loop till program termination
            while(QuitFlag.load())
            {
                
                // Change the PWM signal Duty cycle
                pwmWrite(buzzer_pin, Duty);
                
                // Make delay by the inverse of the frequency
                std::this_thread::sleep_for(std::chrono::milliseconds(1000/freq)); // we put 1,000 at the top since we delay in mSecond
            }
        }
        
        // Destructor
        ~Buzzer()
        {
            
            // Close the PWM signal and make buzzer stop making sound
            pinMode(buzzer_pin, OUTPUT); // Set Buzzer pin as output
            digitalWrite(buzzer_pin, LOW); // Sent low signal to stop the buzzer from making sound
        }
};

// Function to handle shared memory
void shared_memory(void)
{
    
    // Open Shared Memory for Reading Only with all user have read and write permissions
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    
    // We will make memory mapping, we will make casting because the pointer the function returns is a pointer to void
    /*
        1) first zero --> To tell the kernel to choose the suitable first address for the shared memory
        I will not choose the address the kernel will choose it
        
        2) last zero --> we want offset zero --> we tell compiler start from the first address from the address the kernel chose
        for the shared memory
        
        3)MAP_SHARED --> Means that any other process can access this shared memory
        while if it was private (instead of shared) therefore no other process will be able to access this shared memory
    */
    buffer = static_cast<char*>(mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0));
    
    // while loop till the user terminate the program
    while (QuitFlag.load())
    {
        
        // Compare old_buffer with buffer if they are different --> print the new frequency
        if (strcmp(old_buffer, buffer))
        {
            
            // Display the new frequency
            std::cout << "Frequency is " << buffer << std::endl;
            
            // Change the value of the old_buffer
            strcpy(old_buffer, buffer);
        }
        
        // Make some delay as good practice
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
}

int main()
{
    
    // We will setup the Number of pin to be by wiring Pi
    wiringPiSetup();
    
    // Create object of the class buzzer and choose the buzzer pin to be pin 1 in wiring Pi
    Buzzer buzzer_1(1);
    
    // We will make function shared_memory on another thread than main
    std::thread T1(shared_memory);
    
    // We will make run() & set_frequency() in buzzer class each operate on a different thread
    std::thread T2(&Buzzer::set_frequency, &buzzer_1);
    std::thread T3(&Buzzer::run, &buzzer_1);
    
    // To ask User to terminate the program
    std::cout << "Please, Enter any character to quit the program.....\n";
    std::cin.get();
    
    // Change QuitFlag to false to terminate the program
    QuitFlag.store(false);
    
    // Close the shared memory mapping
    munmap(buffer, SHM_SIZE);
    
    // Close shared memory file
    close(shm_fd);
    
    // Wait for the other thread to finish
    if (T1.joinable())
    {
        
        // Wait for the thread to finish
        T1.join();
    }
    if (T2.joinable())
    {
        
        // Wait for the thread to finish
        T2.join();
    }
    if (T3.joinable())
    {
        
        // Wait for the thread to finish
        T3.join();
    }
    
    // Print that Program is Done
    std::cout << "Program is Terminated\n";
    
    return 0;
}