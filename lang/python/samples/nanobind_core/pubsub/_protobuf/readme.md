To generate files for newer protoc / python versions run:

```
..\path\to\protoc.exe --python_out=vX animal.proto house.proto person.proto
```

Python proto files are compatible per major Python version, e.g. with a Protobuf 7 release we need to add a new v7 folder.