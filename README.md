# Moksha

A very simple ncurses IRC client written in C. It is currently unfinished.

# Build

Enter the repository's root directory and run `make`.  The resulting binary
can be found in the `bin` folder.

## Dependencies

- GCC

# Usage

Run `moksha`.

The interface is currently quite barren. The following commands can be used:

| command | argument(s) | description |
| ------------- | -------------- | -------------- |
| `/connect` | `[server]` | Connect to an IRC server. |
| `/disconnect` | `[server]` | Disconnect from an IRC server. |
| `/join` | `[channel]` | Join a channel. |
| `/part` | `[channel]` | Leave a channel. |
| `/quit` |  | Exit Moksha. |


# Planned features

* [ ] Handle multiple IRC servers from the interface
* [ ] Support TLS with BearSSL
* [ ] Handle different user profiles per IRC server
* [ ] A finished UI

# License

```text
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
```
