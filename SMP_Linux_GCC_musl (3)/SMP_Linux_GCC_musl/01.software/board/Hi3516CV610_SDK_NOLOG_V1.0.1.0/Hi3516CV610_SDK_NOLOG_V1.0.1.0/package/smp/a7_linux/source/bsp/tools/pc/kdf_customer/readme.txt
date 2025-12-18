1. tar xf KDF_Linux_SecureBoot_Vx.x.xx.tar.gz
   cd KDF_Linux_SecureBoot_Vx.x.xx
   (Note:version of Vx.x.xx should be based on actual situation)
2. Fill in the relevant data in the file of key.ini
   param_file_path is the path of parameter.bin, output_file_path use for output the key file
3. Execute the command: 
   chmod +x KDF_Linux_SecureBoot
   ./KDF_Linux_SecureBoot ../key.ini
   (Note:The version of Ubantu used must be above 18.04)
