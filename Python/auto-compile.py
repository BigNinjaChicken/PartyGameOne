from pynput import keyboard
from pynput.keyboard import Key, Controller
import pyautogui
import keyboard
import time

# Create a keyboard controller
keyboard_controller = Controller()

# Function to send Alt+Break hotkey
def press_build_button():
    print("Sending Ctrl+Alt+F11")
    keyboard_controller.press(Key.ctrl)
    keyboard_controller.press(Key.alt)
    keyboard_controller.press(Key.f11)
    time.sleep(0.1)
    keyboard_controller.release(Key.f11)
    keyboard_controller.release(Key.alt)
    keyboard_controller.release(Key.ctrl)

# Function to find and click the 'Cancel' button
def click_cancel_button():
    try:
        cancel_location = pyautogui.locateOnScreen('./cancel_button.png', confidence=0.9)
        if cancel_location is not None:
            x, y = pyautogui.center(cancel_location)
            pyautogui.click(x, y)
            print("Clicked 'Cancel' button.")
        else:
            print("Could not find the 'Cancel' button on the screen.")
    except Exception as e:
        print(f"Error: {e}")

def on_key_event(e):
    if e.event_type == keyboard.KEY_DOWN and keyboard.is_pressed('ctrl') and keyboard.is_pressed('s'):
        print("Ctrl + S detected!")
      
        press_build_button()

# Hook the key event listener
keyboard.hook(on_key_event)

# Keep the script running
keyboard.wait('esc')  # You can change this to any other key to exit the script
