class MemWaster
{
public:
  MemWaster();

private:
  char *junk;
};

MemWaster::MemWaster()
{
  junk = (char*) malloc(11);
  for (int n = 0; n < 10; n++) {
    junk[n] = 'a';
  }
  junk[10] = '\0';
}
