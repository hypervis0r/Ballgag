# Ballgag
A tool used to clear Windows Event Logs without invoking `wevtutil.exe`

## Usage
### Remote
`C:\>ballgag.exe <domain> <dc> <username> <password>`

* Domain   - the AD root domain

* DC       - the IP or hostname of the Domain Controller target

* Username - the username of the account used to clear the logs

* Password - the password for said account

### Local
`C:\>ballgag.exe`

* Just run it as admin ;)

## Gotchas

* Account used for clearing logs must be a part of the `Administrators` group.

* If the target machine is remote, the target must have `Remote Event Log Management` enabled in the firewall. 

  If the target machine does not have `Remote Event Log Management` enabled, run locally.
  
## TODO

- [ ] Better argument parsing
- [ ] Pass-the-Hash abilities
- [ ] Clearing specific log channels
- [ ] Save cleared logs to a file
- [ ] Shut the event log service up for good by killing its threads
- [ ] impacket script?
