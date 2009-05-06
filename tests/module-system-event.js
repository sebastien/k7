var buf = new system.event.Buffer();

buf.push8(11);
var eleven = buf.pull8();
print("eleven == "+eleven+"\n");
