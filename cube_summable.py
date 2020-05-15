
import math
import time

'''
The following function will find the number of cube-summable numbers between 1 and a target value B
Input: a positive integer B which represents the target value 
Output: the number of cube-summable values and the time taken for the computation
Time Complexity: O(n^2)
Space Complexity: O(n^2)
'''

def Sub_Cube_Sum(B):

    # check inputs
    if(B <= 0 or not isinstance(B, int)):
        raise Exception("You must pass an integer >= 1")

    ans = 0

    # build array of cubed numbers up to and including the target value
    cubed = []
    maxx = math.floor(B ** (1/3))
    for i in range(1, maxx+1):
        cubed.append(i**3)

    # build out the matrix
    n = len(cubed)
    matrix = []
    for i in range(0, n+1):
        row = []
        for k in range(0, B+1):
            row.append(0)
        matrix.append(row)

    # set base cases - 1 is summable, 0 is not
    for i in range(0, n+1):
        matrix[i][0] = 1

    for i in range(1, B+1):
        matrix[0][i] = 0

    start = time.time()

    # fill out table
    for i in range(1, n+1):
        for j in range(1, B+1):
            if j - cubed[i-1] < 0:
                matrix[i][j] = matrix[i-1][j]
            else:
                matrix[i][j] = max(matrix[i - 1][j], matrix[i - 1][j - cubed[i - 1]] )

    end = time.time()

    # count the number of sum-cube-able numbers
    for i in range(1, B + 1):
        if matrix[n][i] == 1:
            ans += 1

    # return answer
    return ans, end - start

num, time = Sub_Cube_Sum(500000)
print("number of sum-cube-able numbers: ", num)
print("time taken for computation: ", time)
