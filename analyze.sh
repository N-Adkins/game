cmake --build build
CodeChecker analyze ./build/compile_commands.json -o ./build/reports
CodeChecker parse ./build/reports -e html -o ./build/reports_html
firefox ./build/reports_html/index.html
