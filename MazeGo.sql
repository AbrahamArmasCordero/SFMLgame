-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Nov 28, 2018 at 05:54 PM
-- Server version: 5.7.24-0ubuntu0.18.04.1
-- PHP Version: 7.2.10-0ubuntu0.18.04.1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `MazeGo`
--

-- --------------------------------------------------------

--
-- Table structure for table `Maps_Data`
--

CREATE TABLE `Maps_Data` (
  `MapID` int(11) NOT NULL,
  `MapInfo` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `Players_Data`
--

CREATE TABLE `Players_Data` (
  `PlayerID` int(11) NOT NULL,
  `PlayerUser` varchar(21) NOT NULL,
  `PlayerPass` varchar(21) NOT NULL,
  `Gold` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `Pokemon_Data`
--

CREATE TABLE `Pokemon_Data` (
  `PokemonID` int(11) NOT NULL,
  `Name` varchar(21) NOT NULL,
  `Description` text NOT NULL,
  `Type` varchar(21) NOT NULL,
  `GoldForHour` int(11) NOT NULL,
  `SpawnPercentage` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `Pokemon_List`
--

CREATE TABLE `Pokemon_List` (
  `ListID` int(11) NOT NULL,
  `PlayerID_FK` int(11) NOT NULL,
  `PokemonID_FK` int(11) NOT NULL,
  `Quantity` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `Sessions`
--

CREATE TABLE `Sessions` (
  `SesionID` int(11) NOT NULL,
  `PlayerID_FK` int(11) NOT NULL,
  `MapaID_FK` int(11) NOT NULL,
  `ConectionDate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `DesconectionDate` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `Maps_Data`
--
ALTER TABLE `Maps_Data`
  ADD PRIMARY KEY (`MapID`);

--
-- Indexes for table `Players_Data`
--
ALTER TABLE `Players_Data`
  ADD PRIMARY KEY (`PlayerID`),
  ADD UNIQUE KEY `PlayerUser` (`PlayerUser`);

--
-- Indexes for table `Pokemon_Data`
--
ALTER TABLE `Pokemon_Data`
  ADD PRIMARY KEY (`PokemonID`);

--
-- Indexes for table `Pokemon_List`
--
ALTER TABLE `Pokemon_List`
  ADD PRIMARY KEY (`ListID`);

--
-- Indexes for table `Sessions`
--
ALTER TABLE `Sessions`
  ADD PRIMARY KEY (`SesionID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `Maps_Data`
--
ALTER TABLE `Maps_Data`
  MODIFY `MapID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Players_Data`
--
ALTER TABLE `Players_Data`
  MODIFY `PlayerID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=32;
--
-- AUTO_INCREMENT for table `Pokemon_Data`
--
ALTER TABLE `Pokemon_Data`
  MODIFY `PokemonID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- AUTO_INCREMENT for table `Pokemon_List`
--
ALTER TABLE `Pokemon_List`
  MODIFY `ListID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Sessions`
--
ALTER TABLE `Sessions`
  MODIFY `SesionID` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
