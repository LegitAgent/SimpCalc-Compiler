# Compile the C program
gcc main.c parser.c scanner.c -o main_program.exe

# Run the program if compilation succeeds ($LASTEXITCODE tracking)
if ($LASTEXITCODE -eq 0) {
    .\main_program.exe
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
}
