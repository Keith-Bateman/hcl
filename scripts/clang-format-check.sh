#!/bin/bash 

clang-format -h

# Run a command 
find ../test ../include -iname *.h -o -iname *.cpp | xargs clang-format -i -n --Werror 

# Check the exit status of the command 
if [ $? -ne 0 ]; then 
  # The command failed, print an error message 
  echo "The command failed with exit status $?" 
  # Exit the script with a non-zero exit status to indicate failure
  exit 1
else 
  # The command was successful, print a success message 
  echo "The command succeeded with exit status $?" 
fi 