import matplotlib.pyplot as plt
import matplotlib.animation as ani
import pandas as pd
# taking values deapth data csv file
try :
    f = pd.read_csv("Depth Data (1).csv")
# checking for file
except FileNotFoundError:
    print("File not found")
#removing unwanted spaces
f.columns = f.columns.str.strip()
rawval = pd.to_numeric(f["Depth (m)"],errors="coerce")

#removing corrupted values and storing in another list
cleaned_depths= []

for val in rawval:
    if pd.notna(val):
        posval = abs(val)
    else:
        posval =  None

    if posval is None or posval ==0 or posval > 700:
        goodval = cleaned_depths[-1]
    else:
        goodval = posval
    cleaned_depths.append(goodval)
#making a list for timestamps
timestop =list(range(1,len(cleaned_depths)+1))

#code for average values for a smooth graph
smoothdepth= []
for i in range(len(cleaned_depths)):
    #removing the first and last values to not take average of
    if i <2 or i >= len(cleaned_depths)-2:
        smoothdepth.append(cleaned_depths[i])
    else:
    #taking average of 5 data values
        avg = (cleaned_depths[i-2] + cleaned_depths[i-1]+cleaned_depths[i] + cleaned_depths[i+1]+cleaned_depths[i+2])/5
        smoothdepth.append(avg)

#plotting the graph
fig, ax = plt.subplots()
#setting title axis names and having
ax.set_title("Ship Depth Sensor")
ax.set_xlabel("Time (s)")
ax.set_ylabel("Depth (m)")
ax.grid(True)
#setting limits of the axes
ax.set_ylim(max(cleaned_depths)+20 ,0)
ax.set_xlim(1 ,len(timestop))

#lines showing graphs with their attributes
line_raw = ax.plot([],[],color= "green",alpha = 0.5 , label = "Raw Depth", linestyle = "--")[0]
line_smooth = ax.plot([],[],color = "red", alpha = 0.5 , label = "Smooth Depth", linewidth = 2)[0]
#setting up the legend
ax.legend(loc="upper left")


def update(frame):
    line_raw.set_data(timestop[:frame],cleaned_depths[:frame])
    line_smooth.set_data(timestop[:frame], smoothdepth[:frame])
    return line_raw,line_smooth

#making the animation
ani1 = ani.FuncAnimation(fig, update, frames=len(timestop)+1, interval=10, repeat=False)
ani2= ani.FuncAnimation(fig, update, frames=len(timestop)+1, interval=10, repeat=False)
#calling the show attribute to start the graph
plt.show()