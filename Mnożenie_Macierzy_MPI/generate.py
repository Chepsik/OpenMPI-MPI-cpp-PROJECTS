import numpy

random_matrix = numpy.random.randint(10,1000000,(100,100)) / 1000000

numpy.savetxt('matrix_A.csv', random_matrix, delimiter=';', fmt='%s')

random_matrix = numpy.random.randint(10,1000000,(100,100)) / 1000000

numpy.savetxt('matrix_B.csv', random_matrix, delimiter=';', fmt='%s')
