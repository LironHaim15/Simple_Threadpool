# Simple_Threadpool

## About
A small program I had to create as part of Operating Systems course assignment. Implementing a thread pool that accepts jobs and executes them over X number of threads, when X is determined by the user. Each job is assined to an unbusy thread. If the threadpool is full and busy, the job will wait until one of the threads is done and this thread will take on that job. The code is thread safe and memory leak proof.

## Goal
This program's goal was to have a small practice with threading, thread safe code and avoiding memory leaks.