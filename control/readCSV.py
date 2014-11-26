import csv
cr = csv.reader(open("11.24CalibrationData.csv","rb"))

listOfNumbers = []
for row in cr:
	listOfNumbers.append([n for n in row])

print listOfNumbers[0][0]
