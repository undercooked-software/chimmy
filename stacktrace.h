
// TODO: Static Assert
// TODO: Assert Messages

// LINK: https://barrgroup.com/embedded-systems/how-to/define-assert-macro
// LINK: https://stackoverflow.com/a/5252544
#define AssertBreak(Expression) (*((int*)0) = 0)
#define AssertAlways(Expression) \
  Statement( if (!(Expression)) { AssertBreak(Expression); } )

#if !SHIP_BUILD
  #define Assert(Expression) AssertAlways(Expression)
#else
  #define Assert(Expression)
#endif