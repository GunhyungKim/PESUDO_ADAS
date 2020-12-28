from random import randint, uniform, choice
from time import sleep

def gps_rand():
    sleep(1)
    N = randint(0, 10) #nuber of objects
    name = ["person", "car", "dog", "cat"]
    Locations = [{"lat":uniform(35.886, 35.887), "lng":uniform(128.609, 128.610), "name":choice(name), "azimuth":45}]
    count = 0
    while count < N:
        count += 1
        Locations.append({"lat":uniform(35.886, 35.887), "lng":uniform(128.609, 128.610), "name":choice(name)})
        
    return Locations
