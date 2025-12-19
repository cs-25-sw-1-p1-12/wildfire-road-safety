# Build a separate test executable
add_executable(test_f
        test.c
)

# Register it as a test (optional but correct)
add_test(NAME test_f COMMAND test_f)
