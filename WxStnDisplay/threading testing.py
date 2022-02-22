import logging
import threading
import time

def thread_function(name):
    logging.info("Thread %s: starting", name)
    time.sleep(2)
    logging.info("Thread %s: finishing", name)

if __name__ == "__main__":
    format="%(asctime)s: %(message)s"
    logging.basicConfig(format=format, level=logging.INFO, datefmt="%H:%M:%S")
    
    logging.info("Main     : before creating thread")
    x=threading.Thread(target=thread_function , args=(1,), daemon=True)
    x.start()
    logging.info("Main     : wait for the thread to finish")
    logging.info("Main     : all done")