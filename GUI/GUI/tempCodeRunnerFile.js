fs.appendFile("Output.txt", msg[1], (err) => {
//   if (err) {
//     console.log(err);
//   }
//   else {
//     // Get the file contents after the append operation
//     console.log("\nFile Contents of file after append:",
//       fs.readFileSync("example_file.txt", "utf8"));
//   }
// });