cmake_minimum_required(VERSION 3.22.0)

project(sqlite)
add_library(${PROJECT_NAME} sqlite3.c)
